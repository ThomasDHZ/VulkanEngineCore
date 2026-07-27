using System.Reflection;
using System.Runtime.InteropServices;

namespace VulkanEngineCoreCS
{
    public static class DLLSystem
    {
        private static bool                     _initialized = false;
        private static string?                  _dllDirectory;
        private static readonly HashSet<string> _loadedDlls = new(StringComparer.OrdinalIgnoreCase);

        public static void Initialize(string? customDllFolder = null)
        {
            if (_initialized) return;

            _dllDirectory = string.IsNullOrEmpty(customDllFolder) ? Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location)! : Path.GetFullPath(customDllFolder);
            if (!Directory.Exists(_dllDirectory)) throw new DirectoryNotFoundException($"DLL directory not found: {_dllDirectory}");
            NativeLibrary.SetDllImportResolver(Assembly.GetExecutingAssembly(), DllImportResolver);
            _initialized = true;
        }

        private static IntPtr DllImportResolver(string libraryName, Assembly assembly, DllImportSearchPath? searchPath)
        {
            string fullPath = Path.Combine(_dllDirectory!, libraryName);

            if (!fullPath.EndsWith(".dll", StringComparison.OrdinalIgnoreCase)) fullPath += ".dll";
            if (_loadedDlls.Contains(fullPath)) return IntPtr.Zero;
            if (File.Exists(fullPath))
            {
                IntPtr handle = NativeLibrary.Load(fullPath);
                if (handle != IntPtr.Zero)
                {
                    _loadedDlls.Add(fullPath);
                    return handle;
                }
            }
            return IntPtr.Zero;
        }

        public static bool IsDllLoaded(string dllName)
        {
            string fullPath = Path.Combine(_dllDirectory ?? "", dllName);
            if (!fullPath.EndsWith(".dll", StringComparison.OrdinalIgnoreCase))
                fullPath += ".dll";

            return _loadedDlls.Contains(fullPath);
        }

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
    }
}