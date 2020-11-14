#pragma once
#include "Project/Base.h"

#include <vector>

namespace df
{
	enum class VertexAttribType;

	class VertexBuffer
	{
	public:
		VertexBuffer(ulong size);
		VertexBuffer(const void* data, ulong size);
		~VertexBuffer();

		void SetData(const void* data, ulong size) const;

		void SetLayout(const std::vector<VertexAttribType>& layout) const;

		void Bind() const;
		void Unbind() const;

		uint GetVAO() const { return VAO; }
		uint GetVBO() const { return VBO; }

	private:
		uint VAO, VBO;
		bool m_isStatic = false;

		ulong _GetAttribSize(VertexAttribType layout) const;
	};

	enum class VertexAttribType
	{
		Float, Float2, Float3, Float4,
		Int, Int2, Int3, Int4,
		Texture, Byte
	};
}
