#include "hzpch.h"
#include "Renderer.h"
#include "FrameBuffer.h"
#include "Platform/OpenGL/OpenGLFrameBuffer.h"
namespace Hazel
{
	Ref<FrameBuffer> FrameBuffer::Create(const FrameBufferSpecification& spec)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:		HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr; nullptr;
		case RendererAPI::API::OpenGL:		return std::make_shared<OpenGLFrameBuffer>(spec);
		}

		HZ_CORE_ASSERT(false, "Unknown rendererAPI!");
		return nullptr;
	}
}