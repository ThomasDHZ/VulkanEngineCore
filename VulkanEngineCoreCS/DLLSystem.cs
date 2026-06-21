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
        private static string ExeDirectory => Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location)!;
        private static string EngineRoot => Path.GetFullPath(Path.Combine(ExeDirectory, @"..\..\..\..\"));
        public static readonly string VulkanEngineCorePath = Path.Combine(EngineRoot, @"x64\Debug\VulkanEngineCoreInterlopDLL.dll");

        [DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Auto)] private static extern IntPtr LoadLibrary(string lpFileName);
        [DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Auto)] private static extern bool SetDllDirectory(string lpPathName);

        public static void SetSharedDllDirectory()
        {
            string dllDir = Path.GetDirectoryName(VulkanEngineCorePath)!;
            Directory.SetCurrentDirectory(Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "..//..//..//..//Assets"));
            if (!Directory.Exists(dllDir)) throw new DirectoryNotFoundException($"DLL folder missing: {dllDir}");
            if (!SetDllDirectory(dllDir)) throw new System.ComponentModel.Win32Exception(Marshal.GetLastWin32Error());
            if (LoadLibrary(VulkanEngineCorePath) == IntPtr.Zero) throw new System.ComponentModel.Win32Exception(Marshal.GetLastWin32Error());
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
    }
}
