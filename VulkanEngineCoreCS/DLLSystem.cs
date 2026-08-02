using System.Reflection;
using System.Runtime.InteropServices;

namespace VulkanEngineCoreCS
{
    public static class DLLSystem
    {
        private static bool _initialized = false;
        private static string _dllDirectory = "";
        private static readonly HashSet<string> _loadedDlls = new(StringComparer.OrdinalIgnoreCase);

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
        [DllImport("kernel32", SetLastError = true)] private static extern bool SetDllDirectory(string lpPathName);
    }
}