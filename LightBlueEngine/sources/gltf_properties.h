#ifndef __GLTF_PROPERTIES_H__
#define __GLTF_PROPERTIES_H__

// <>インクルード
#include <tiny_gltf.h>

// namespace >> [glTFProperties]
// 参考：https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html
namespace glTFProperties
{
	// [5.1] struct >> glTFProperties >> [Accessor]
	struct Accessor
	{
		// フェッチ
		static void Fetch(tinygltf::Model model, std::vector<Accessor>& container)
		{
			for (auto& accessor : model.accessors)
			{
				auto& cont_accessor = container.emplace_back();
				cont_accessor.buffer_view		= accessor.bufferView;
				cont_accessor.byte_offset		= accessor.byteOffset;
				cont_accessor.component_type	= accessor.componentType;
				cont_accessor.normalized		= accessor.normalized;
				cont_accessor.count				= accessor.count;
				cont_accessor.type				= accessor.type;

				for (auto& prm : accessor.maxValues)
				{
					cont_accessor.max.push_back(SCast(float, prm));
				}

				for (auto& prm : accessor.minValues)
				{
					cont_accessor.min.push_back(SCast(float, prm));
				}

				cont_accessor.sparse.count					= accessor.sparse.count;
				cont_accessor.sparse.indices.buffer_view	= accessor.sparse.indices.bufferView;
				cont_accessor.sparse.indices.byte_offset	= accessor.sparse.indices.byteOffset;
				cont_accessor.sparse.indices.component_type	= accessor.sparse.indices.componentType;
				cont_accessor.sparse.values.buffer_view		= accessor.sparse.values.bufferView;
				cont_accessor.sparse.values.byte_offset		= accessor.sparse.values.byteOffset;

				cont_accessor.name = accessor.name;
			}
		}

		// [5.2] struct >> glTFProperties >> Accessor >> [Sparse]
		struct Sparse
		{
			// [5.3] struct >> glTFProperties >> Accessor >> Sparse >> [Indices]
			struct Indices
			{
				int buffer_view		= -1;
				int byte_offset		= 0;
				int component_type	= -1;
			};

			// [5.4] struct >> glTFProperties >> Accessor >> Sparse >> [Values]
			struct Values
			{
				int buffer_view = -1;
				int byte_offset = 0;
			};

			int		count	= -1;
			Indices indices = {};
			Values	values	= {};
		};

		int					buffer_view;
		size_t				byte_offset		= 0;
		int					component_type	= -1;
		bool				normalized		= false;
		int					count			= -1;
		std::string			type;
		std::vector<float>	max;
		std::vector<float>	min;
		Sparse				sparse;
		std::string			name;
	};

	// [5.5] struct >> glTFProperties >> [Animation]
	struct Animation
	{
		static void Fetch(tinygltf::Model model, std::vector<Animation>& container)
		{
			for (auto& animation : model.animations)
			{
				auto& cont_animation = container.emplace_back();

				for (auto& prm : animation.channels)
				{
					auto& cont = cont_animation.channels.emplace_back();
					cont.samplaer = prm.sampler;
					cont.target.node = prm.target_node;
					cont.target.path = prm.target_path;
				}

				for (auto& prm : animation.samplers)
				{
					auto& cont = cont_animation.samplers.emplace_back();
					cont.input			= prm.input;
					cont.interpolation	= prm.interpolation;
					cont.output			= prm.output;
				}

				cont_animation.name = animation.name;
			}
		}

		// [5.6] struct >> glTFProperties >> Animation >> [Channel]
		struct Channel
		{
			// [5.7] struct >> glTFProperties >> Animation >> Channel >> Target
			struct Target
			{
				int			node = 0;
				std::string path;
			};

			int		samplaer	= 0;
			Target	target		= {};
		};

		// [5.8] struct >> glTFProperties >> Animation >> [Sampler]
		struct Sampler
		{
			int			input;
			std::string interpolation = "LINEAR";
			int			output;
		};

		std::vector<Channel>	channels;
		std::vector<Sampler>	samplers;
		std::string				name;
	};

	// [5.9] struct >> glTFProperties >> [Asset]
	struct Asset
	{
		static void Fetch(tinygltf::Model model, Asset& object)
		{
			object.copyright = model.asset.copyright;
			object.generator = model.asset.generator;
			object.version = model.asset.version;
			object.min_version = model.asset.minVersion;
		}
			
		std::string copyright;
		std::string generator;
		std::string version;
		std::string min_version;
	};

	// [5.10] struct >> glTFProperties >> [Buffer]
	struct Buffer
	{
		static void Fetch(tinygltf::Model model, std::vector<Buffer>& container)
		{
			for (auto& buffer : model.buffers)
			{
				auto& cont_buffer = container.emplace_back();
				
				cont_buffer.uri			= buffer.uri;
				cont_buffer.byte_length = buffer.data.size();
				cont_buffer.name		= buffer.name;
			}
		}

		std::string uri;
		size_t		byte_length;
		std::string name;
	};

	// [5.11] struct >> glTFProperties >> [BufferView]
	struct BufferView
	{
		static void Fetch(tinygltf::Model model, std::vector<BufferView>& container)
		{
			for (auto& buffer_view : model.bufferViews)
			{
				auto& cont_buffer_view = container.emplace_back();
				cont_buffer_view.buffer			= buffer_view.buffer;
				cont_buffer_view.byte_offset	= buffer_view.byteOffset;
				cont_buffer_view.byte_length	= buffer_view.byteLength;
				cont_buffer_view.byte_stride	= buffer_view.byteStride;
				cont_buffer_view.target			= buffer_view.target;
				cont_buffer_view.name			= buffer_view.name;
			}
		}

		int			buffer;
		size_t		byte_offset = 0;
		size_t		byte_length;
		size_t		byte_stride;
		int			target;
		std::string name;
	};

	// [5.12] struct >> glTFProperties >> [Camera]
	struct Camera
	{
		static void Fetch(tinygltf::Model model, std::vector<Camera>& container)
		{
			for (auto& camera : model.cameras)
			{
				auto& cont_camera = container.emplace_back();

				cont_camera.type = camera.type;
				
				if(cont_camera.type == "orthographic")
				{
					cont_camera.orthograpic.xmag	= SCast(float, camera.orthographic.xmag);
					cont_camera.orthograpic.ymag	= SCast(float, camera.orthographic.ymag);
					cont_camera.orthograpic.zfar	= SCast(float, camera.orthographic.zfar);
					cont_camera.orthograpic.znear	= SCast(float, camera.orthographic.znear);
				}
				else // "perspective"
				{
					cont_camera.perspective.aspect_ratio	= SCast(float, camera.perspective.aspectRatio);
					cont_camera.perspective.yfov			= SCast(float, camera.perspective.yfov);
					cont_camera.perspective.zfar			= SCast(float, camera.perspective.zfar);
					cont_camera.perspective.znear			= SCast(float, camera.perspective.znear);
				}

				cont_camera.name = camera.name;
			}
		}

		// [5.13] struct >> glTFProperties >> Camera >> [Orthographic]
		struct Orthographic
		{
			float xmag;
			float ymag;
			float zfar;
			float znear;
		};

		// [5.13] struct >> glTFProperties >> Camera >> [Perspective]
		struct Perspective
		{
			float aspect_ratio;
			float yfov;
			float zfar;
			float znear;
		};

		Orthographic	orthograpic;
		Perspective		perspective;
		std::string		type;
		std::string		name;
	};
}

#endif // __GLTF_PROPERTIES_H__

