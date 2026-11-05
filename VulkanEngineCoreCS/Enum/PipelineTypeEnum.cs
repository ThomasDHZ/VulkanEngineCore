using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VulkanEngineCoreCS.Enum
{
    public enum PipelineTypeEnum : uint
    {
        DefaultPipeline,
        DepthPipeline,
        WireFramePipeline,
        ReflectionPipeline,
        CollisionPipeline
    };
}
