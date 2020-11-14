#include "dfpch.h"
#include "VertexBuffer.h"

#include <glad/glad.h>

namespace df
{
	VertexBuffer::VertexBuffer(ulong size): m_isStatic(false) {
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(float) * 5, 0);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(float) * 5, (void*)(sizeof(float) * 3));

	}

	VertexBuffer::VertexBuffer(const void* data, ulong size) {
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(float) * 5, 0);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(float) * 5, (void*)(sizeof(float) * 3));

	}

	VertexBuffer::~VertexBuffer() {
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
	}

	void VertexBuffer::SetData(const void* data, ulong size) const {
		Bind();
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
	}


	ulong VertexBuffer::_GetAttribSize(VertexAttribType attrib) const {
		switch (attrib) {
			case df::VertexAttribType::Float:
			return sizeof(float);
			case df::VertexAttribType::Float2:
			return sizeof(float) * 2;
			case df::VertexAttribType::Float3:
			return sizeof(float) * 3;
			case df::VertexAttribType::Float4:
			return sizeof(float) * 4;

			case df::VertexAttribType::Int:
			case df::VertexAttribType::Int2:
			case df::VertexAttribType::Int3:
			case df::VertexAttribType::Int4:
			return sizeof(int);

			case df::VertexAttribType::Texture:
			return sizeof(int);

			case df::VertexAttribType::Byte:
			return sizeof(char);
		}
	}

	void VertexBuffer::SetLayout(const std::vector<VertexAttribType>& layout) const {
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		ulong size = [&]() {
			ulong size = 0;
			for(auto attrib : layout)
				size += _GetAttribSize(attrib);
			return size;
		} ();
		ulong stride = 0;
		for (uint i = 0; i < layout.size(); i++) {
			glEnableVertexAttribArray(i);
			auto attrib = layout[i];
			switch (attrib) {
				case VertexAttribType::Float:
					glVertexAttribPointer(i, 1, GL_FLOAT, false, size, (void*)stride);
				break;
				case VertexAttribType::Float2:
					glVertexAttribPointer(i, 2, GL_FLOAT, false, size, (void*)stride);
				break;
				case VertexAttribType::Float3:
					glVertexAttribPointer(i, 3, GL_FLOAT, false, size, (void*)stride);
				break;
				case VertexAttribType::Float4:
					glVertexAttribPointer(i, 4, GL_FLOAT, false, size, (void*)stride);
				break;
			}

			stride += _GetAttribSize(attrib);
		}
	}

	void VertexBuffer::Bind() const {
		glBindVertexArray(VAO);
	}

	void VertexBuffer::Unbind() const {
		glBindVertexArray(0);
	}

}