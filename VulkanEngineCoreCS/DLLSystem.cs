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
        private static bool _sharedDirSet = false;
        private static string ExeDirectory => Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location)!;
        private static string EngineRoot => Path.GetFullPath(Path.Combine(ExeDirectory, @"..\..\..\..\"));
        public static readonly string VulkanEngineCorePath = Path.Combine(EngineRoot, @"x64\Debug\VulkanEngineCoreInterlopDLL.dll");

        [DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Auto)] private static extern IntPtr LoadLibrary(string lpFileName);
        [DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Auto)] private static extern bool SetDllDirectory(string lpPathName);
        [DllImport("VulkanEngineCoreInterlopDLL.dll", CallingConvention = CallingConvention.StdCall)] private static extern void DebugSystem_SetRootDirectory([MarshalAs(UnmanagedType.LPStr)] string engineRoot);

        public static void SetSharedDllDirectory()
        {
            if (_sharedDirSet) return;

            string dllDir = Path.GetDirectoryName(VulkanEngineCorePath)!;

            if (!Directory.Exists(dllDir)) throw new DirectoryNotFoundException($"DLL folder missing: {dllDir}");
            if (!SetDllDirectory(dllDir)) throw new System.ComponentModel.Win32Exception(Marshal.GetLastWin32Error());

            DebugSystem_SetRootDirectory("..\\..\\..\\..\\Assets");

            IntPtr handle = LoadLibrary(VulkanEngineCorePath);
            if (handle == IntPtr.Zero) throw new System.ComponentModel.Win32Exception(Marshal.GetLastWin32Error());
            _sharedDirSet = true;
        }

        public static void CallDLLFunc(Action action)
        {
            if (!_sharedDirSet)
            {
                throw new InvalidOperationException("Call SetSharedDllDirectory() first!");
            }

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
            if (!_sharedDirSet)
            {
                throw new InvalidOperationException("Call SetSharedDllDirectory() first!");
            }

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
    }
}
