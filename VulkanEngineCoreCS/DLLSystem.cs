using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace VulkanEngineCoreCS
{
    public static class DLLSystem
    {
        private static string? _dllDirectory;
        private static bool    _initialized = false;

        public static void Initialize(string? customDllFolder = null)
        {
            if (_initialized) return;

            if (!string.IsNullOrEmpty(customDllFolder)) _dllDirectory = Path.GetFullPath(customDllFolder);
            else _dllDirectory = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location)!;
            if (!Directory.Exists(_dllDirectory))
            {
                throw new DirectoryNotFoundException($"DLL directory not found: {_dllDirectory}");
            }

            NativeLibrary.SetDllImportResolver(Assembly.GetExecutingAssembly(), DllImportResolver);
            _initialized = true;
        }

        private static IntPtr DllImportResolver(string libraryName, Assembly assembly, DllImportSearchPath? searchPath)
        {
            string fullPath = Path.Combine(_dllDirectory!, libraryName);
            if (!fullPath.EndsWith(".dll", StringComparison.OrdinalIgnoreCase)) fullPath += ".dll";
            if (File.Exists(fullPath)) return NativeLibrary.Load(fullPath);
            return IntPtr.Zero;
        }


        public static void CallDLLFunc(Action action)
        {
            try
            {
                action();
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.ToString());
            }
        }

        public static TResult CallDLLFunc<TResult>(Func<TResult> func)
        {
            try
            {
                TResult result = func();
                if (typeof(TResult) == typeof(IntPtr))
                {
                    IntPtr ptr = (IntPtr)(object)result;
                    if (ptr == IntPtr.Zero)
                    {
                        return default;
                    }
                    TResult copyResult = Marshal.PtrToStructure<TResult>(ptr);
                    return copyResult;
                }
                return result;
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.ToString());
                return default(TResult);
            }
        }
        [DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Auto)] private static extern IntPtr LoadLibrary(string lpFileName);
        [DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Auto)] private static extern bool SetDllDirectory(string lpPathName);
    }
}
