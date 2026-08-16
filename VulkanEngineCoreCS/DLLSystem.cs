using System.Reflection;
using System.Runtime.InteropServices;
using System;
using System.Collections.Generic;
using System.IO;
using System.Text;

namespace VulkanEngineCoreCS
{
    public sealed class NativeExport
    {
        public string Name { get; init; } = "";
        public uint Ordinal { get; init; }
        public uint FunctionRva { get; init; }
        public string RvaHex => $"0x{FunctionRva:X8}";
    }


    public static class DLLSystem
    {
        private static bool _initialized = false;
        private static string _dllDirectory = "";
        private static readonly HashSet<string> _loadedDlls = new(StringComparer.OrdinalIgnoreCase);
        private static ushort ReadU16(byte[] f, int o) => BitConverter.ToUInt16(f, o);
        private static uint ReadU32(byte[] f, int o) => BitConverter.ToUInt32(f, o);
        public static void Initialize(string? customDllFolder = null)
        {
            if (_initialized) return;

            _dllDirectory = string.IsNullOrEmpty(customDllFolder) ? Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location)! : Path.GetFullPath(customDllFolder);
            if (!Directory.Exists(_dllDirectory)) throw new DirectoryNotFoundException($"DLL directory not found: {_dllDirectory}");

            SetDllDirectory(_dllDirectory);
            NativeLibrary.SetDllImportResolver(Assembly.GetExecutingAssembly(), DllImportResolver);

            _initialized = true;
        }

        private static IntPtr DllImportResolver(string libraryName, Assembly assembly, DllImportSearchPath? searchPath)
        {
            string dllPath = Path.Combine(_dllDirectory, libraryName);
            if (!dllPath.EndsWith(".dll", StringComparison.OrdinalIgnoreCase)) dllPath += ".dll";
            if (File.Exists(dllPath))
            {
                if (NativeLibrary.TryLoad(dllPath, out IntPtr handle))
                {
                    _loadedDlls.Add(dllPath);
                    return handle;
                }
            }
            return IntPtr.Zero;
        }

        public static bool IsDllLoaded(string dllName) => _loadedDlls.Contains(Path.Combine(_dllDirectory, dllName));
        public static IReadOnlyCollection<string> GetLoadedDlls() => _loadedDlls;

        public static void CallDLLFunc(Action action)
        {
            try
            {
                action();
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex);
            }
        }

        public static T CallDLLFunc<T>(Func<T> func)
        {
            try
            {
                return func();
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex);
                return default!;
            }
        }

        public static List<NativeExport> ListDllExport(string dllPath)
        {
            var result = new List<NativeExport>();
            byte[] file = File.ReadAllBytes(dllPath);
            if(file.Length < 0x40 ||
                ReadU16(file, 0) != 0x5A4D)
            {
                throw new InvalidDataException("Not a PE file (missing MZ).");
            }

            int portableExecutableOffset = (int)ReadU32(file, 0x3C);
            if (portableExecutableOffset + 4 > file.Length || 
                ReadU32(file, portableExecutableOffset) != 0x00004550)
            {
                throw new InvalidDataException("Not a PE file (missing portable executable signature).");
            }

            int    fileHeader = portableExecutableOffset + 4;
            ushort machine = ReadU16(file, fileHeader + 0);
            ushort numberOfSections = ReadU16(file, fileHeader + 2);
            ushort sizeOfOptionalHeader = ReadU16(file, fileHeader + 16);
            int    optionalHeader = fileHeader + 20;

            ushort magicHeader = ReadU16(file, optionalHeader + 0);
            if(magicHeader != 0x20B)
            {
                throw new NotSupportedException("Only x64 DLLs supported.");
            }

            int exportDirRva = (int)ReadU32(file, optionalHeader + 112);
            int exportDirSize = (int)ReadU32(file, optionalHeader + 116);
            if (exportDirRva == 0 || 
                exportDirSize == 0)
            {
                return result;
            }

            int sectionTable = optionalHeader + sizeOfOptionalHeader;
            int exportOffset = RvaToOffset(file, sectionTable, numberOfSections, exportDirRva);
            if (exportOffset < 0)
            {
                throw new InvalidDataException("Could not map export directory RVA.");
            }

            uint numberOfFunctions  = ReadU32(file, exportOffset + 20);
            uint numberOfNames      = ReadU32(file, exportOffset + 24);
            uint addressOfFunctions = ReadU32(file, exportOffset + 28);
            uint addressOfNames     = ReadU32(file, exportOffset + 32);
            uint addressOfOrdinals  = ReadU32(file, exportOffset + 36);
            uint ordinalBase        = ReadU32(file, exportOffset + 16);

            int namesOffset = RvaToOffset(file, sectionTable, numberOfSections, (int)addressOfNames);
            int ordsOffset  = RvaToOffset(file, sectionTable, numberOfSections, (int)addressOfOrdinals);
            int funcsOffset = RvaToOffset(file, sectionTable, numberOfSections, (int)addressOfFunctions);
            if (namesOffset < 0 || ordsOffset < 0 || funcsOffset < 0)
            {
                throw new InvalidDataException("Could not map export name/ordinal tables.");
            }

            for (uint x = 0; x < numberOfNames; x++)
            {
                int nameRva = (int)ReadU32(file, namesOffset + (int)(x * 4));
                int nameOff = RvaToOffset(file, sectionTable, numberOfSections, nameRva);
                if (nameOff < 0) continue;

                string name = ReadAnsiZ(file, nameOff);
                ushort ordinalIndex = ReadU16(file, ordsOffset + (int)(x * 2));
                uint funcRva = ReadU32(file, funcsOffset + ordinalIndex * 4);
                uint ordinal = ordinalBase + ordinalIndex;

                result.Add(new NativeExport
                {
                    Name = name,
                    Ordinal = ordinal,
                    FunctionRva = funcRva
                });
            }

            result.Sort((a, b) => string.CompareOrdinal(a.Name, b.Name));
            return result;
        }

        private static int RvaToOffset(byte[] file, int sectionTable, int numberOfSections, int rva)
        {
            for (int x = 0; x < numberOfSections; x++)
            {
                int sec = sectionTable + x * 40;
                int virtAddr = (int)ReadU32(file, sec + 12);
                int virtSize = (int)ReadU32(file, sec + 8);
                int rawPtr   = (int)ReadU32(file, sec + 20);
                int rawSize  = (int)ReadU32(file, sec + 16);

                int size = Math.Max(virtSize, rawSize);
                if (rva >= virtAddr && rva < virtAddr + size)
                {
                    return rawPtr + (rva - virtAddr);
                }
            }
            return -1;
        }

        private static string ReadAnsiZ(byte[] file, int offset)
        {
            int end = offset;
            while (end < file.Length && file[end] != 0) end++;
            return Encoding.ASCII.GetString(file, offset, end - offset);
        }

        [DllImport("kernel32", SetLastError = true)] private static extern bool SetDllDirectory(string lpPathName);
    }
}