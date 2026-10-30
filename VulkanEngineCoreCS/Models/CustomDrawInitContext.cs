using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VulkanEngineCoreCS.Models
{
    public struct CustomDrawInitContext
    {

        public Guid RenderPassId { get; set; } = Guid.Empty;
        public Guid TextureAttachmentId { get; set; } = Guid.Empty;
        public float TileSize { get; set; }
        public CustomDrawInitContext()
        {
        }
    };
}
