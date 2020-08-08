#pragma once
#include "Hazel/Renderer/FrameBuffer.h"

namespace Hazel
{
	class OpenGLFrameBuffer : public FrameBuffer
	{
	public:
		OpenGLFrameBuffer(const FrameBufferSpecification& spec);
		virtual ~OpenGLFrameBuffer();

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual const FrameBufferSpecification& GetSpecification() const override { return m_Spec; }
		virtual uint32_t GetColorAttachmentRendererID() const override { return m_ColorAttachment; }

		void Invalidate();
	private:
		uint32_t m_RendererID;
		uint32_t m_ColorAttachment, m_DepthAttachment;
		FrameBufferSpecification m_Spec;
	};
}