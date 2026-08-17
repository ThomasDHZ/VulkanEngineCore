using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace VulkanEngineCoreCS.Vulkan
{
    public unsafe class ListPtr<T> : IEnumerable<T> where T : struct
    {
        private readonly T* _ptr;
        private readonly size_t _count;

        public size_t Count => _count;
        public uint UCount => (uint)_count;
        public T* Ptr => _ptr;

        public bool IsEmpty => _count == 0 || _ptr == null;

        public ListPtr(T* ptr, size_t count)
        {
            _ptr = ptr;
            _count = count;
        }

        public ListPtr() : this(null, 0) { }

        public T this[size_t index]
        {
            get
            {
                if (index >= _count)
                    throw new IndexOutOfRangeException($"Index {index} is out of range (Count = {_count})");
                return _ptr[index];
            }
            set
            {
                if (index >= _count)
                    throw new IndexOutOfRangeException($"Index {index} is out of range (Count = {_count})");
                _ptr[index] = value;
            }
        }

        public IEnumerator<T> GetEnumerator()
        {
            for (size_t i = 0; i < _count; i++)
            {
                yield return this[i];
            }
        }

        IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();

        public List<T> ToList()
        {
            var list = new List<T>((int)_count);
            for (size_t i = 0; i < _count; i++)
            {
                list.Add(_ptr[i]);
            }
            return list;
        }

        public override string ToString()
        {
            return $"ListPtr<{typeof(T).Name}> Count={_count} Ptr=0x{(ulong)_ptr:X}";
        }
    }
}
