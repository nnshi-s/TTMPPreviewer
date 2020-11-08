/*
 * The Source Code is Straightly Modified From BGFX/Example 18: IBL
 * https://github.com/bkaradzic/bgfx/tree/master/examples/18-ibl
 * 
 */

#include <vector>
#include <string>
#include <memory>
#include <filesystem>
#include <thread>
#include <mutex>
#include <future>

#include "common.h"
#include "bgfx_utils.h"
#include "imgui/imgui.h"
#include "nanovg/nanovg.h"

#include <bx/readerwriter.h>
#include <bx/string.h>

#include <Model.h>
#include <Tex.h>
#include <Ttmp.h>
#include <Mpl.h>


#include <Windows.h>

namespace
{
	class BgfxMesh;

	static float s_texelHalf = 0.0f;

	struct Uniforms
	{
		enum { NumVec4 = 12 };

		void init()
		{
			u_params = bgfx::createUniform("u_params", bgfx::UniformType::Vec4, NumVec4);
		}

		void submit()
		{
			bgfx::setUniform(u_params, m_params, NumVec4);
		}

		void destroy()
		{
			bgfx::destroy(u_params);
		}

		union
		{
			struct
			{
				union
				{
					float m_mtx[16];

					/* 0*/
					struct
					{
						float m_mtx0[4];
					};

					/* 1*/
					struct
					{
						float m_mtx1[4];
					};

					/* 2*/
					struct
					{
						float m_mtx2[4];
					};

					/* 3*/
					struct
					{
						float m_mtx3[4];
					};
				};

				/* 4*/
				struct
				{
					float m_glossiness, m_reflectivity, m_exposure, m_bgType;
				};

				/* 5*/
				struct
				{
					float m_metalOrSpec, m_unused5[3];
				};

				/* 6*/
				struct
				{
					float m_doDiffuse, m_doSpecular, m_doDiffuseIbl, m_doSpecularIbl;
				};

				/* 7*/
				struct
				{
					float m_cameraPos[3], m_unused7[1];
				};

				/* 8*/
				struct
				{
					float m_rgbDiff[4];
				};

				/* 9*/
				struct
				{
					float m_rgbSpec[4];
				};

				/*10*/
				struct
				{
					float m_lightDir[3], m_unused10[1];
				};

				/*11*/
				struct
				{
					float m_lightCol[3], m_unused11[1];
				};
			};

			float m_params[NumVec4 * 4];
		};

		bgfx::UniformHandle u_params;
	};

	struct PosColorTexCoord0Vertex
	{
		// position
		float m_x;
		float m_y;
		float m_z;
		// vertex color
		uint32_t m_rgba;
		// texture coordinate
		float m_u;
		float m_v;
		// normal
		float m_nx;
		float m_ny;
		float m_nz;
		// tangent
		float m_tx;
		float m_ty;
		float m_tz;
		// bitangent
		float m_bx;
		float m_by;
		float m_bz;

		static void init()
		{
			ms_layout
				.begin()
				.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
				.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
				.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
				.add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
				.add(bgfx::Attrib::Tangent, 3, bgfx::AttribType::Float)
				.add(bgfx::Attrib::Bitangent, 3, bgfx::AttribType::Float)
				.end();
		}

		static bgfx::VertexLayout ms_layout;
	};

	bgfx::VertexLayout PosColorTexCoord0Vertex::ms_layout;

	bool operator==(const PosColorTexCoord0Vertex& p1, const PosColorTexCoord0Vertex& p2)
	{
		char* p_p1 = (char*)&p1;
		char* p_p2 = (char*)&p2;
		if (sizeof(p1) != sizeof(p2))
			return false;
		for (int i = 0; i != sizeof(p1); ++i)
		{
			if (*p_p1 != *p_p2)
				return false;
			p_p1++;
			p_p2++;
		}
		return true;
	}

	void screenSpaceQuad(float _textureWidth, float _textureHeight, bool _originBottomLeft = false, float _width = 1.0f,
	                     float _height = 1.0f)
	{
		if (3 == bgfx::getAvailTransientVertexBuffer(3, PosColorTexCoord0Vertex::ms_layout))
		{
			bgfx::TransientVertexBuffer vb;
			bgfx::allocTransientVertexBuffer(&vb, 3, PosColorTexCoord0Vertex::ms_layout);
			PosColorTexCoord0Vertex* vertex = (PosColorTexCoord0Vertex*)vb.data;

			const float zz = 0.0f;

			const float minx = -_width;
			const float maxx = _width;
			const float miny = 0.0f;
			const float maxy = _height * 2.0f;

			const float texelHalfW = s_texelHalf / _textureWidth;
			const float texelHalfH = s_texelHalf / _textureHeight;
			const float minu = -1.0f + texelHalfW;
			const float maxu = 1.0f + texelHalfW;

			float minv = texelHalfH;
			float maxv = 2.0f + texelHalfH;

			if (_originBottomLeft)
			{
				std::swap(minv, maxv);
				minv -= 1.0f;
				maxv -= 1.0f;
			}

			vertex[0].m_x = minx;
			vertex[0].m_y = miny;
			vertex[0].m_z = zz;
			vertex[0].m_rgba = 0xffffffff;
			vertex[0].m_u = minu;
			vertex[0].m_v = minv;

			vertex[1].m_x = maxx;
			vertex[1].m_y = miny;
			vertex[1].m_z = zz;
			vertex[1].m_rgba = 0xffffffff;
			vertex[1].m_u = maxu;
			vertex[1].m_v = minv;

			vertex[2].m_x = maxx;
			vertex[2].m_y = maxy;
			vertex[2].m_z = zz;
			vertex[2].m_rgba = 0xffffffff;
			vertex[2].m_u = maxu;
			vertex[2].m_v = maxv;

			bgfx::setVertexBuffer(0, &vb);
		}
	}

	struct LightProbe
	{
		enum Enum
		{
			Bolonga,
			Kyoto,

			Count
		};

		void load(const char* _name)
		{
			char filePath[512];

			bx::snprintf(filePath, BX_COUNTOF(filePath), "textures/%s_lod.dds", _name);
			m_tex = loadTexture(filePath, BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_W_CLAMP);

			bx::snprintf(filePath, BX_COUNTOF(filePath), "textures/%s_irr.dds", _name);
			m_texIrr = loadTexture(filePath, BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_W_CLAMP);
		}

		void destroy()
		{
			bgfx::destroy(m_tex);
			bgfx::destroy(m_texIrr);
		}

		bgfx::TextureHandle m_tex;
		bgfx::TextureHandle m_texIrr;
	};

	struct Camera
	{
		Camera()
		{
			reset();
		}

		void reset()
		{
			m_target.curr = {0.0f, 0.0f, 0.0f};
			m_target.dest = {0.0f, 0.0f, 0.0f};

			m_pos.curr = {0.0f, 0.0f, -3.0f};
			m_pos.dest = {0.0f, 0.0f, -3.0f};

			m_orbit[0] = 0.0f;
			m_orbit[1] = 0.0f;
		}

		void mtxLookAt(float* _outViewMtx)
		{
			bx::mtxLookAt(_outViewMtx, m_pos.curr, m_target.curr);
		}

		void orbit(float _dx, float _dy)
		{
			m_orbit[0] += _dx;
			m_orbit[1] += _dy;
		}

		void dolly(float _dz)
		{
			const float cnear = 1.0f;
			const float cfar = 100.0f;

			const bx::Vec3 toTarget = bx::sub(m_target.dest, m_pos.dest);
			const float toTargetLen = bx::length(toTarget);
			const float invToTargetLen = 1.0f / (toTargetLen + bx::kFloatMin);
			const bx::Vec3 toTargetNorm = bx::mul(toTarget, invToTargetLen);

			float delta = toTargetLen * _dz;
			float newLen = toTargetLen + delta;
			if ((cnear < newLen || _dz < 0.0f)
				&& (newLen < cfar || _dz > 0.0f))
			{
				m_pos.dest = bx::mad(toTargetNorm, delta, m_pos.dest);
			}
		}

		void consumeOrbit(float _amount)
		{
			float consume[2];
			consume[0] = m_orbit[0] * _amount;
			consume[1] = m_orbit[1] * _amount;
			m_orbit[0] -= consume[0];
			m_orbit[1] -= consume[1];

			const bx::Vec3 toPos = bx::sub(m_pos.curr, m_target.curr);
			const float toPosLen = bx::length(toPos);
			const float invToPosLen = 1.0f / (toPosLen + bx::kFloatMin);
			const bx::Vec3 toPosNorm = bx::mul(toPos, invToPosLen);

			float ll[2];
			bx::toLatLong(&ll[0], &ll[1], toPosNorm);
			ll[0] += consume[0];
			ll[1] -= consume[1];
			ll[1] = bx::clamp(ll[1], 0.02f, 0.98f);

			const bx::Vec3 tmp = bx::fromLatLong(ll[0], ll[1]);
			const bx::Vec3 diff = bx::mul(bx::sub(tmp, toPosNorm), toPosLen);

			m_pos.curr = bx::add(m_pos.curr, diff);
			m_pos.dest = bx::add(m_pos.dest, diff);
		}

		void update(float _dt)
		{
			const float amount = bx::min(_dt / 0.12f, 1.0f);

			consumeOrbit(amount);

			m_target.curr = bx::lerp(m_target.curr, m_target.dest, amount);
			m_pos.curr = bx::lerp(m_pos.curr, m_pos.dest, amount);
		}

		void envViewMtx(float* _mtx)
		{
			const bx::Vec3 toTarget = bx::sub(m_target.curr, m_pos.curr);
			const float toTargetLen = bx::length(toTarget);
			const float invToTargetLen = 1.0f / (toTargetLen + bx::kFloatMin);
			const bx::Vec3 toTargetNorm = bx::mul(toTarget, invToTargetLen);

			const bx::Vec3 right = bx::normalize(bx::cross({0.0f, 1.0f, 0.0f}, toTargetNorm));
			const bx::Vec3 up = bx::normalize(bx::cross(toTargetNorm, right));

			_mtx[0] = right.x;
			_mtx[1] = right.y;
			_mtx[2] = right.z;
			_mtx[3] = 0.0f;
			_mtx[4] = up.x;
			_mtx[5] = up.y;
			_mtx[6] = up.z;
			_mtx[7] = 0.0f;
			_mtx[8] = toTargetNorm.x;
			_mtx[9] = toTargetNorm.y;
			_mtx[10] = toTargetNorm.z;
			_mtx[11] = 0.0f;
			_mtx[12] = 0.0f;
			_mtx[13] = 0.0f;
			_mtx[14] = 0.0f;
			_mtx[15] = 1.0f;
		}

		struct Interp3f
		{
			bx::Vec3 curr;
			bx::Vec3 dest;
		};

		Interp3f m_target;
		Interp3f m_pos;
		float m_orbit[2];
	};

	struct Mouse
	{
		Mouse()
			: m_dx(0.0f)
			  , m_dy(0.0f)
			  , m_prevMx(0.0f)
			  , m_prevMy(0.0f)
			  , m_scroll(0)
			  , m_scrollPrev(0)
		{
		}

		void update(float _mx, float _my, int32_t _mz, uint32_t _width, uint32_t _height)
		{
			const float widthf = float(int32_t(_width));
			const float heightf = float(int32_t(_height));

			// Delta movement.
			m_dx = float(_mx - m_prevMx) / widthf;
			m_dy = float(_my - m_prevMy) / heightf;

			m_prevMx = _mx;
			m_prevMy = _my;

			// Scroll.
			m_scroll = _mz - m_scrollPrev;
			m_scrollPrev = _mz;
		}

		float m_dx; // Screen space.
		float m_dy;
		float m_prevMx;
		float m_prevMy;
		int32_t m_scroll;
		int32_t m_scrollPrev;
	};

	struct Settings
	{
		Settings()
		{
			m_envRotCurr = 0.0f;
			m_envRotDest = 0.0f;
			m_lightDir[0] = -0.8f;
			m_lightDir[1] = 0.2f;
			m_lightDir[2] = -0.5f;
			m_lightCol[0] = 1.0f;
			m_lightCol[1] = 1.0f;
			m_lightCol[2] = 1.0f;
			m_glossiness = 0.7f;
			m_exposure = 0.0f;
			m_bgType = 3.0f;
			m_radianceSlider = 2.0f;
			m_reflectivity = 0.85f;
			m_rgbDiff[0] = 1.0f;
			m_rgbDiff[1] = 1.0f;
			m_rgbDiff[2] = 1.0f;
			m_rgbSpec[0] = 1.0f;
			m_rgbSpec[1] = 1.0f;
			m_rgbSpec[2] = 1.0f;
			m_lod = 0.0f;
			m_doDiffuse = false;
			m_doSpecular = false;
			m_doDiffuseIbl = true;
			m_doSpecularIbl = true;
			m_showLightColorWheel = true;
			m_showDiffColorWheel = true;
			m_showSpecColorWheel = true;
			m_metalOrSpec = 0;
			m_meshSelection = 0;
		}

		float m_envRotCurr;
		float m_envRotDest;
		float m_lightDir[3];
		float m_lightCol[3];
		float m_glossiness;
		float m_exposure;
		float m_radianceSlider;
		float m_bgType;
		float m_reflectivity;
		float m_rgbDiff[3];
		float m_rgbSpec[3];
		float m_lod;
		bool m_doDiffuse;
		bool m_doSpecular;
		bool m_doDiffuseIbl;
		bool m_doSpecularIbl;
		bool m_showLightColorWheel;
		bool m_showDiffColorWheel;
		bool m_showSpecColorWheel;
		int32_t m_metalOrSpec;
		int32_t m_meshSelection;
	};

	class BgfxMesh
	{
	public:
		std::vector<PosColorTexCoord0Vertex> mVertices;
		std::vector<unsigned int> mIndices;
		Tt::Tex mDiffuse;
		Tt::Tex mNormal;
		Tt::Tex mSpecular;

		bgfx::VertexBufferHandle mVbh;
		bgfx::IndexBufferHandle mIbh;
		bgfx::TextureHandle mDth;
		bgfx::TextureHandle mNth;
		bgfx::TextureHandle mSth;

		void bindHandles(); // create bgfx handles and bind to buffer & texture

		std::tuple<bgfx::VertexBufferHandle,
		           bgfx::IndexBufferHandle,
		           bgfx::TextureHandle,
		           bgfx::TextureHandle,
		           bgfx::TextureHandle> getHandles(); // return handles

		bool isBody = false;

		static const std::map<const Tt::TexFormat, const bgfx::TextureFormat::Enum> mPixFormatMap;
	};

	const std::map<const Tt::TexFormat, const bgfx::TextureFormat::Enum> BgfxMesh::mPixFormatMap = {
		{Tt::TexFormat::L8, bgfx::TextureFormat::R8},
		{Tt::TexFormat::A8, bgfx::TextureFormat::A8},
		{Tt::TexFormat::A4R4G4B4, bgfx::TextureFormat::RGBA4},
		{Tt::TexFormat::A1R5G5B5, bgfx::TextureFormat::RGB5A1},
		{Tt::TexFormat::A8R8G8B8, bgfx::TextureFormat::BGRA8},
		{Tt::TexFormat::X8R8G8B8, bgfx::TextureFormat::RGBA8},
		{Tt::TexFormat::G16R16F, bgfx::TextureFormat::RG16F},
		{Tt::TexFormat::G32R32F, bgfx::TextureFormat::RG32F},
		{Tt::TexFormat::A16B16G16R16F, bgfx::TextureFormat::RGBA16F},
		{Tt::TexFormat::A32B32G32R32F, bgfx::TextureFormat::RGBA32F},
		{Tt::TexFormat::DXT1, bgfx::TextureFormat::BC1},
		{Tt::TexFormat::DXT3, bgfx::TextureFormat::BC2},
		{Tt::TexFormat::DXT5, bgfx::TextureFormat::BC3},
		{Tt::TexFormat::D16, bgfx::TextureFormat::D16},
		{Tt::TexFormat::INVALID, bgfx::TextureFormat::Unknown},
	};

	void BgfxMesh::bindHandles()
	{
		// bind vertex buffer
		mVbh = bgfx::createVertexBuffer(
			bgfx::makeRef(mVertices.data(),
			              mVertices.size() * sizeof(PosColorTexCoord0Vertex)),
			PosColorTexCoord0Vertex::ms_layout
		);


		// bind index buffer
		mIbh = bgfx::createIndexBuffer(
			bgfx::makeRef(mIndices.data(),
			              mIndices.size() * sizeof(unsigned int)),
			BGFX_BUFFER_INDEX32 // important
		);


		// bind diffuse map
		if (!mDiffuse.mpDecompressedDataRaw) // if the mesh has no diffuse map
		{
			if (isBody)
				mDth = loadTexture(".\\textures\\c0201b0001_d.dds");
			else
				mDth = loadTexture(".\\textures\\default_diffuse.dds");
		}
		else
		{
			mDth = bgfx::createTexture2D(
				mDiffuse.mWidth,
				mDiffuse.mHeight,
				false,
				0,
				mPixFormatMap.at(mDiffuse.mTexFormat),
				0,
				bgfx::makeRef(mDiffuse.mpDecompressedDataRaw, mDiffuse.getMip0TextureSize())
			);
		}


		// bind normal map
		if (!mNormal.mpDecompressedDataRaw) // if the mesh has no diffuse map
		{
			if (isBody)
				mNth = loadTexture(".\\textures\\c0201b0001_n.dds");
			else
				mNth = loadTexture(".\\textures\\default_normal.dds");
		}
		else
		{
			mNth = bgfx::createTexture2D(
				mNormal.mWidth,
				mNormal.mHeight,
				false,
				0,
				mPixFormatMap.at(mNormal.mTexFormat),
				0,
				bgfx::makeRef(mNormal.mpDecompressedDataRaw, mNormal.getMip0TextureSize())
			);
		}


		// bind specular map
		if (!mSpecular.mpDecompressedDataRaw) // if the mesh has no diffuse map
		{
			mSth = loadTexture(".\\textures\\default_specular.dds");
		}
		else
		{
			mSth = bgfx::createTexture2D(
				mSpecular.mWidth,
				mSpecular.mHeight,
				false,
				0,
				mPixFormatMap.at(mSpecular.mTexFormat),
				0,
				bgfx::makeRef(mSpecular.mpDecompressedDataRaw, mSpecular.getMip0TextureSize())
			);
		}
	}

	std::tuple<bgfx::VertexBufferHandle, bgfx::IndexBufferHandle, bgfx::TextureHandle,
	           bgfx::TextureHandle, bgfx::TextureHandle> BgfxMesh::getHandles()
	{
		return {mVbh, mIbh, mDth, mNth, mSth};
	}


	class TTMPViewer : public entry::AppI
	{
	public:
		TTMPViewer(const char* _name, const char* _description, const char* _url)
			: entry::AppI(_name, _description, _url), mMeshCount(0)
		{
		}

		void init(int32_t _argc, const char* const* _argv, uint32_t _width, uint32_t _height) override
		{
			Args args(_argc, _argv);

			mMeshCount = 0;
			mWidth = _width;
			mHeight = _height;
			mDebug = BGFX_DEBUG_NONE;
			mReset = 0
				| BGFX_RESET_VSYNC
				| BGFX_RESET_MSAA_X16;

			bgfx::Init init;
			// init.type = args.m_type;
			init.type = bgfx::RendererType::Direct3D11;
			init.vendorId = args.m_pciId;
			init.resolution.width = mWidth;
			init.resolution.height = mHeight;
			init.resolution.reset = mReset;
			bgfx::init(init);

			// Enable debug text.
			bgfx::setDebug(mDebug);

			// Set views  clear state.
			bgfx::setViewClear(0
			                   , BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
			                   , 0x303030ff
			                   , 1.0f
			                   , 0
			);

			// Imgui.
			imguiCreate();

			// Uniforms.
			mUniforms.init();

			// Vertex declarations.
			PosColorTexCoord0Vertex::init();

			mLightProbes[LightProbe::Bolonga].load("bolonga");
			mLightProbes[LightProbe::Kyoto].load("kyoto");
			mCurrentLightProbe = LightProbe::Bolonga;

			uMtx = bgfx::createUniform("u_mtx", bgfx::UniformType::Mat4);
			uParams = bgfx::createUniform("u_params", bgfx::UniformType::Vec4);
			uFlags = bgfx::createUniform("u_flags", bgfx::UniformType::Vec4);
			uCamPos = bgfx::createUniform("u_camPos", bgfx::UniformType::Vec4);
			sTexCube = bgfx::createUniform("s_texCube", bgfx::UniformType::Sampler);
			sTexCubeIrr = bgfx::createUniform("s_texCubeIrr", bgfx::UniformType::Sampler);
			sTexDiffuse = bgfx::createUniform("s_texDiff", bgfx::UniformType::Sampler);
			sTexNormal = bgfx::createUniform("s_texNormal", bgfx::UniformType::Sampler);
			sTexSpecular = bgfx::createUniform("s_texSpecular", bgfx::UniformType::Sampler);

			mProgramMesh = loadProgram("vs_ibl_mesh", "fs_ibl_mesh");
			mProgramSky = loadProgram("vs_ibl_skybox", "fs_ibl_skybox");


			// set light
			/*mSettings.m_doDiffuse = true;
			mSettings.m_doSpecular = true;
			mSettings.m_lightDir[0] = -0.5f;
			mSettings.m_lightDir[1] = 0.0f;
			mSettings.m_lightDir[2] = -0.1f;*/
			mSettings.m_doDiffuseIbl = true;
			mSettings.m_doSpecularIbl = true;

			// set initial modpack name
			mCurrentModpackNameC = std::make_unique<char[]>(8);
			memcpy(mCurrentModpackNameC.get(), "ModPack", 8);
		}

		virtual int shutdown() override
		{

			// Cleanup.
			bgfx::destroy(mProgramMesh);
			bgfx::destroy(mProgramSky);

			bgfx::destroy(uCamPos);
			bgfx::destroy(uFlags);
			bgfx::destroy(uParams);
			bgfx::destroy(uMtx);

			bgfx::destroy(sTexCube);
			bgfx::destroy(sTexCubeIrr);

			for (uint8_t ii = 0; ii < LightProbe::Count; ++ii)
			{
				mLightProbes[ii].destroy();
			}

			mUniforms.destroy();

			imguiDestroy();

			// Shutdown bgfx.
			bgfx::shutdown();

			// clear the /tmp folder
			std::filesystem::path tmp(".\\tmp");
			std::filesystem::remove_all(tmp);

			return 0;
		}

		bool update() override
		{
			if (!entry::processEvents(mWidth, mHeight, mDebug, mReset, mDropFilePath, &mMouseState))
			{
				imguiBeginFrame(mMouseState.m_mx
				                , mMouseState.m_my
				                , (mMouseState.m_buttons[entry::MouseButton::Left] ? IMGUI_MBUT_LEFT : 0)
				                | (mMouseState.m_buttons[entry::MouseButton::Right] ? IMGUI_MBUT_RIGHT : 0)
				                | (mMouseState.m_buttons[entry::MouseButton::Middle] ? IMGUI_MBUT_MIDDLE : 0)
				                , mMouseState.m_mz
				                , uint16_t(mWidth)
				                , uint16_t(mHeight)
				);

				showExampleDialog(this);


				ImGui::SetNextWindowPos(
					ImVec2(mWidth - mWidth / 5.0f - 10.0f, 10.0f)
					, ImGuiCond_FirstUseEver
				);
				ImGui::SetNextWindowSize(
					ImVec2(mWidth / 5.0f, mHeight - 20.0f)
					, ImGuiCond_FirstUseEver
				);
				ImGui::Begin("Settings"
				             , NULL
				             , 0
				);
				ImGui::PushItemWidth(180.0f);

				/*ImGui::Text("Environment light:");
				ImGui::Indent();
				ImGui::Checkbox("IBL Diffuse", &mSettings.m_doDiffuseIbl);
				ImGui::Checkbox("IBL Specular", &mSettings.m_doSpecularIbl);*/

				if (ImGui::BeginTabBar("Cubemap", ImGuiTabBarFlags_None))
				{
					if (ImGui::BeginTabItem("Bolonga"))
					{
						mCurrentLightProbe = LightProbe::Bolonga;
						ImGui::EndTabItem();
					}

					if (ImGui::BeginTabItem("Kyoto"))
					{
						mCurrentLightProbe = LightProbe::Kyoto;
						ImGui::EndTabItem();
					}

					ImGui::EndTabBar();
				}

				ImGui::SliderFloat("^v", &mTransformY, -2.0f, 2.0f);
				ImGui::SliderFloat("-+", &mTransformScale, 0.3f, 3.0f);

				/*ImGui::SliderFloat("Texture LOD", &mSettings.m_lod, 0.0f, 10.1f);
				ImGui::Unindent();*/

				ImGui::Separator();
				ImGui::Text("Directional light:");
				ImGui::Indent();
				ImGui::Checkbox("Diffuse", &mSettings.m_doDiffuse);
				ImGui::Checkbox("Specular", &mSettings.m_doSpecular);
				const bool doDirectLighting = mSettings.m_doDiffuse || mSettings.m_doSpecular;
				if (doDirectLighting)
				{
					ImGui::SliderFloat("Light direction X", &mSettings.m_lightDir[0], -1.0f, 1.0f);
					ImGui::SliderFloat("Light direction Y", &mSettings.m_lightDir[1], -1.0f, 1.0f);
					ImGui::SliderFloat("Light direction Z", &mSettings.m_lightDir[2], -1.0f, 1.0f);
					ImGui::ColorWheel("Color:", mSettings.m_lightCol, 0.6f);
				}
				ImGui::Unindent();

				ImGui::Separator();
				/*ImGui::Text("Background:");
				ImGui::Indent();
				{
					if (ImGui::BeginTabBar("CubemapSelection", ImGuiTabBarFlags_None))
					{
						if (ImGui::BeginTabItem("Irradiance"))
						{
							mSettings.m_bgType = mSettings.m_radianceSlider;
							ImGui::EndTabItem();
						}

						if (ImGui::BeginTabItem("Radiance"))
						{
							mSettings.m_bgType = 7.0f;

							ImGui::SliderFloat("Mip level", &mSettings.m_radianceSlider, 1.0f, 6.0f);

							ImGui::EndTabItem();
						}

						if (ImGui::BeginTabItem("Skybox"))
						{
							mSettings.m_bgType = 0.0f;
							ImGui::EndTabItem();
						}

						ImGui::EndTabBar();
					}
				}
				ImGui::Unindent();

				ImGui::Separator();
				ImGui::Text("Post processing:");
				ImGui::Indent();
				ImGui::SliderFloat("Exposure", &mSettings.m_exposure, -4.0f, 4.0f);
				ImGui::Unindent();*/

				ImGui::PopItemWidth();
				ImGui::End();

				ImGui::SetNextWindowPos(
					ImVec2(10.0f, 260.0f)
					, ImGuiCond_FirstUseEver
				);
				ImGui::SetNextWindowSize(
					ImVec2(mWidth / 5.0f, 450.0f)
					, ImGuiCond_FirstUseEver
				);
				/*ImGui::Begin("Mesh"
				             , NULL
				             , 0
				);

				ImGui::Text("Mesh:");
				ImGui::Indent();
				ImGui::RadioButton("Bunny", &mSettings.m_meshSelection, 0);
				ImGui::RadioButton("Orbs", &mSettings.m_meshSelection, 1);
				ImGui::Unindent();

				const bool isBunny = (0 == mSettings.m_meshSelection);
				if (!isBunny)
				{
					mSettings.m_metalOrSpec = 0;
				}
				else
				{
					ImGui::Separator();
					ImGui::Text("Workflow:");
					ImGui::Indent();
					ImGui::RadioButton("Metalness", &mSettings.m_metalOrSpec, 0);
					ImGui::RadioButton("Specular", &mSettings.m_metalOrSpec, 1);
					ImGui::Unindent();

					ImGui::Separator();
					ImGui::Text("Material:");
					ImGui::Indent();
					ImGui::PushItemWidth(130.0f);
					ImGui::SliderFloat("Glossiness", &mSettings.m_glossiness, 0.0f, 1.0f);
					ImGui::SliderFloat(0 == mSettings.m_metalOrSpec ? "Metalness" : "Diffuse - Specular",
					                   &mSettings.m_reflectivity, 0.0f, 1.0f);
					ImGui::PopItemWidth();
					ImGui::Unindent();
				}


				ImGui::ColorWheel("Diffuse:", &mSettings.m_rgbDiff[0], 0.7f);
				ImGui::Separator();
				if ((1 == mSettings.m_metalOrSpec) && isBunny)
				{
					ImGui::ColorWheel("Specular:", &mSettings.m_rgbSpec[0], 0.7f);
				}

				ImGui::End();*/

				ImGui::Begin(mCurrentModpackNameC.get());


				// Tree Node
				if (mMpl.getType() == Tt::MplType::PAGED)
				{
					for (const auto& group : mMpl.mGroups)
					{
						if (ImGui::TreeNode(group.mGroupName.c_str()))
						{
							for (const auto& option : group.mOptions)
							{
								if (ImGui::TreeNode(option.mOptionName.c_str()))
								{
									for (const auto& moddedItem : option.mModdedItems)
									{
										ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf |
											ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet;

										if (&moddedItem == mPreviousSelectedItem)
											flags |= ImGuiTreeNodeFlags_Selected;

										ImGui::TreeNodeEx(moddedItem.mModdedItemName.c_str(), flags);
										if (ImGui::IsItemClicked())
										{
											// modded item selected
											if (!mLoading)
											{
												mLoading = true;


												auto [mdlOff, mdlSize] = moddedItem.getMdlOffSize();
												auto [diffOff, diffSize] = moddedItem.getDiffuseOffSize();
												auto [normOff, normSize] = moddedItem.getNormalOffSize();
												auto [specOff, specSize] = moddedItem.getSpecularOffSize();

												std::thread loadTh(&TTMPViewer::loadThread, this,
												                   mdlOff, mdlSize, diffOff, diffSize,
												                   normOff, normSize, specOff, specSize);

												loadTh.detach();
											}

											mPreviousSelectedItem = &moddedItem;
										}
									}

									ImGui::TreePop();
								}
							}

							ImGui::TreePop();
						}
					}
				}
				else if (mMpl.getType() == Tt::MplType::SIMPLE || mMpl.getType() == Tt::MplType::ITEM)
				{
					for (const auto& moddedItem : mMpl.mModdedItems)
					{
						ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf |
							ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet;

						if (&moddedItem == mPreviousSelectedItem)
							flags |= ImGuiTreeNodeFlags_Selected;

						ImGui::TreeNodeEx(moddedItem.mModdedItemName.c_str(), flags);
						if (ImGui::IsItemClicked())
						{
							// modded item selected
							if (!mLoading)
							{
								mLoading = true;


								auto [mdlOff, mdlSize] = moddedItem.getMdlOffSize();
								auto [diffOff, diffSize] = moddedItem.getDiffuseOffSize();
								auto [normOff, normSize] = moddedItem.getNormalOffSize();
								auto [specOff, specSize] = moddedItem.getSpecularOffSize();

								std::thread loadTh(&TTMPViewer::loadThread, this,
								                   mdlOff, mdlSize, diffOff, diffSize,
								                   normOff, normSize, specOff, specSize);

								loadTh.detach();
							}

							mPreviousSelectedItem = &moddedItem;
						}
					}
				}

				ImGui::End();

				imguiEndFrame();

				mUniforms.m_glossiness = mSettings.m_glossiness;
				mUniforms.m_reflectivity = mSettings.m_reflectivity;
				mUniforms.m_exposure = mSettings.m_exposure;
				mUniforms.m_bgType = mSettings.m_bgType;
				mUniforms.m_metalOrSpec = float(mSettings.m_metalOrSpec);
				mUniforms.m_doDiffuse = float(mSettings.m_doDiffuse);
				mUniforms.m_doSpecular = float(mSettings.m_doSpecular);
				mUniforms.m_doDiffuseIbl = float(mSettings.m_doDiffuseIbl);
				mUniforms.m_doSpecularIbl = float(mSettings.m_doSpecularIbl);
				bx::memCopy(mUniforms.m_rgbDiff, mSettings.m_rgbDiff, 3 * sizeof(float));
				bx::memCopy(mUniforms.m_rgbSpec, mSettings.m_rgbSpec, 3 * sizeof(float));
				bx::memCopy(mUniforms.m_lightDir, mSettings.m_lightDir, 3 * sizeof(float));
				bx::memCopy(mUniforms.m_lightCol, mSettings.m_lightCol, 3 * sizeof(float));

				int64_t now = bx::getHPCounter();
				static int64_t last = now;
				const int64_t frameTime = now - last;
				last = now;
				const double freq = double(bx::getHPFrequency());
				const float deltaTimeSec = float(double(frameTime) / freq);

				// Camera.
				const bool mouseOverGui = ImGui::MouseOverArea();
				mMouse.update(float(mMouseState.m_mx), float(mMouseState.m_my), mMouseState.m_mz, mWidth, mHeight);
				if (!mouseOverGui)
				{
					if (mMouseState.m_buttons[entry::MouseButton::Left])
					{
						mCamera.orbit(mMouse.m_dx, mMouse.m_dy);
					}
					else if (mMouseState.m_buttons[entry::MouseButton::Right])
					{
						mCamera.dolly(mMouse.m_dx + mMouse.m_dy);
					}
					else if (mMouseState.m_buttons[entry::MouseButton::Middle])
					{
						mSettings.m_envRotDest += mMouse.m_dx * 2.0f;
					}
					else if (0 != mMouse.m_scroll)
					{
						mCamera.dolly(float(mMouse.m_scroll) * 0.05f);
					}
				}
				mCamera.update(deltaTimeSec);
				bx::memCopy(mUniforms.m_cameraPos, &mCamera.m_pos.curr.x, 3 * sizeof(float));

				// View Transform 0.
				float view[16];
				bx::mtxIdentity(view);

				const bgfx::Caps* caps = bgfx::getCaps();

				float proj[16];
				bx::mtxOrtho(proj, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 100.0f, 0.0, caps->homogeneousDepth);
				bgfx::setViewTransform(0, view, proj);

				// View Transform 1.
				mCamera.mtxLookAt(view);
				bx::mtxProj(proj, 45.0f, float(mWidth) / float(mHeight), 0.1f, 100.0f, caps->homogeneousDepth);
				bgfx::setViewTransform(1, view, proj);

				// View rect.
				bgfx::setViewRect(0, 0, 0, uint16_t(mWidth), uint16_t(mHeight));
				bgfx::setViewRect(1, 0, 0, uint16_t(mWidth), uint16_t(mHeight));

				// Env rotation.
				const float amount = bx::min(deltaTimeSec / 0.12f, 1.0f);
				mSettings.m_envRotCurr = bx::lerp(mSettings.m_envRotCurr, mSettings.m_envRotDest, amount);

				// Env mtx.
				float mtxEnvView[16];
				mCamera.envViewMtx(mtxEnvView);
				float mtxEnvRot[16];
				bx::mtxRotateY(mtxEnvRot, mSettings.m_envRotCurr);
				bx::mtxMul(mUniforms.m_mtx, mtxEnvView, mtxEnvRot); // Used for Skybox.

				// Submit view 0.
				bgfx::setTexture(0, sTexCube, mLightProbes[mCurrentLightProbe].m_tex);
				bgfx::setTexture(1, sTexCubeIrr, mLightProbes[mCurrentLightProbe].m_texIrr);
				bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
				screenSpaceQuad((float)mWidth, (float)mHeight, true);
				mUniforms.submit();
				bgfx::submit(0, mProgramSky);

				// Submit view 1.
				bx::memCopy(mUniforms.m_mtx, mtxEnvRot, 16 * sizeof(float)); // Used for IBL.

				float mtx[16];
				// bx::mtxSRT(mtx, 1.0f, 1.0f, 1.0f, 0.0f, bx::kPi, 0.0f, 0.0f, -0.80f, 0.0f);
				bx::mtxSRT(mtx, mTransformScale, mTransformScale, mTransformScale,
				           0.0f, bx::kPi, 0.0f,
				           0.0f, mTransformY, 0.0f);

				/*bgfx::setTexture(0, sTexCube, mLightProbes[mCurrentLightProbe].m_tex);
				bgfx::setTexture(1, sTexCubeIrr, mLightProbes[mCurrentLightProbe].m_texIrr);
				mUniforms.submit();
				meshSubmit(mMeshBunny, 1, mProgramMesh, mtx);*/

				mRefreshingMeshesMutex.lock();
				for (auto& mesh : mMeshes)
				{
					bgfx::setVertexBuffer(0, mesh.mVbh);
					bgfx::setIndexBuffer(mesh.mIbh);
					bgfx::setTransform(mtx);
					bgfx::setTexture(0, sTexCube, mLightProbes[mCurrentLightProbe].m_tex);
					bgfx::setTexture(1, sTexCubeIrr, mLightProbes[mCurrentLightProbe].m_texIrr);
					bgfx::setTexture(2, sTexDiffuse, mesh.mDth);
					bgfx::setTexture(3, sTexNormal, mesh.mNth);
					bgfx::setTexture(4, sTexSpecular, mesh.mSth);
					bgfx::setState(BGFX_STATE_DEFAULT | BGFX_STATE_MSAA);
					mUniforms.submit();
					bgfx::submit(1, mProgramMesh);
				}
				mRefreshingMeshesMutex.unlock();


				// Advance to next frame. Rendering thread will be kicked to
				// process submitted rendering primitives.
				bgfx::frame();

				// when some file dragged into the window
				if (!mDropFilePath.empty())
				{
					if (!mUnzipping)
					{
						mUnzipping = true;
						std::thread unzipTh(&TTMPViewer::unzipThread, this);
						unzipTh.detach();
					}
				}

				// when unzipping completes
				if (mUnzipping && mUnzipComplete)
				{
					mDropFilePath.clear();
					mUnzipComplete = false;
					mUnzipping = false;
				}

				// when loading completes
				if (mLoading && mLoadComplete)
				{
					mLoading = false;
					mLoadComplete = false;
				}

				return true;
			}

			return false;
		}

		void unzipThread();
		void loadThread(unsigned mdlOff, unsigned mdlSize,
		                unsigned diffOff, unsigned diffSize,
		                unsigned normOff, unsigned normSize,
		                unsigned specOff, unsigned specSize);
		bool mUnzipping = false;
		bool mUnzipComplete = false; // set by unzip thread
		bool mLoading = false;
		bool mLoadComplete = false; // set by load thread
		std::mutex mRefreshingMeshesMutex;
		const Tt::MplModdedItem* mPreviousSelectedItem = nullptr;

		float mTransformY = -0.8f;
		float mTransformScale = 1.0f;


		uint32_t mWidth;
		uint32_t mHeight;
		uint32_t mDebug;
		uint32_t mReset;
		uint32_t mMeshCount = 0;
		entry::MouseState mMouseState;

		Uniforms mUniforms;

		LightProbe mLightProbes[LightProbe::Count];
		LightProbe::Enum mCurrentLightProbe;

		bgfx::UniformHandle uMtx;
		bgfx::UniformHandle uParams;
		bgfx::UniformHandle uFlags;
		bgfx::UniformHandle uCamPos;
		bgfx::UniformHandle sTexCube;
		bgfx::UniformHandle sTexCubeIrr;

		bgfx::ProgramHandle mProgramMesh;
		bgfx::ProgramHandle mProgramSky;


		Camera mCamera;
		Mouse mMouse;

		Settings mSettings;

		std::wstring mDropFilePath;
		std::wstring mCurrentModpackName;
		std::unique_ptr<char[]> mCurrentModpackNameC;
		Tt::Mpl mMpl;
		std::vector<BgfxMesh> mMeshes;

		bgfx::UniformHandle sTexDiffuse;
		bgfx::UniformHandle sTexNormal;
		bgfx::UniformHandle sTexSpecular;
	};

	static inline void calcTanAndBitan(PosColorTexCoord0Vertex& vertex,
	                                   const PosColorTexCoord0Vertex& vertexConnected0,
	                                   const PosColorTexCoord0Vertex& vertexConnected1)
	{
		// skip if already calculated
		if (vertex.m_tx != 0.0f || vertex.m_ty != 0.0f || vertex.m_tz != 0.0f)
			return;

		// adopted from https://learnopengl.com/Advanced-Lighting/Normal-Mapping
		bx::Vec3 edge1{
			vertexConnected0.m_x - vertex.m_x, vertexConnected0.m_y - vertex.m_y, vertexConnected0.m_z - vertex.m_z
		};
		bx::Vec3 edge2{
			vertexConnected1.m_x - vertex.m_x, vertexConnected1.m_y - vertex.m_y, vertexConnected1.m_z - vertex.m_z
		};

		// use vec3.xy instead of vec2
		bx::Vec3 deltaUV1{vertexConnected0.m_u - vertex.m_u, vertexConnected0.m_v - vertex.m_v, 0.0f};
		bx::Vec3 deltaUV2{vertexConnected1.m_u - vertex.m_u, vertexConnected1.m_v - vertex.m_v, 0.0f};

		float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		vertex.m_tx = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		vertex.m_ty = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		vertex.m_tz = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

		vertex.m_bx = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		vertex.m_by = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		vertex.m_bz = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
	}

	void TTMPViewer::unzipThread()
	{
		Tt::Ttmp ttmp(mDropFilePath);
		if (ttmp.unzip())
		{
			auto beg = mDropFilePath.find_last_of('\\');
			auto end = mDropFilePath.find_last_of('.');
			mCurrentModpackName = mDropFilePath.substr(beg + 1, end - beg - 1);
			mCurrentModpackNameC = std::make_unique<char[]>(128);
			WideCharToMultiByte(CP_UTF8, 0, mCurrentModpackName.c_str(), -1, mCurrentModpackNameC.get(), 128, NULL,
			                    NULL);

			std::wstring mplPath = L".\\tmp\\";
			mplPath += (mCurrentModpackName + L"\\TTMPL.mpl");
			mMpl.load(mplPath);	
		}
		
		mUnzipComplete = true;
	}

	static std::vector<BgfxMesh> loadModelThread(const std::wstring& mpdPath,
	                                             unsigned offset, unsigned size)
	{
		std::vector<BgfxMesh> meshVec;

		std::ifstream ifs;
		Tt::Mdl mdl;
		if (!mdl.readRawFromDisk(ifs, mpdPath, offset, size))
		{
			std::wcerr << "[Error]Could not open " << mpdPath << std::endl;
			return meshVec;
		}
		mdl.loadFromRaw();

		Tt::Model model;
		model.loadFromMdl(mdl);

		// successfully read mdl, create  meshes.
		int16_t meshCount = model.mLoDList[0].meshCount;

		meshVec.reserve(meshCount);
		for (int i = 0; i != meshCount; ++i)
		{
			meshVec.push_back(BgfxMesh());
		}



		for (unsigned short i = 0; i != meshCount; ++i)
		{
			std::vector<PosColorTexCoord0Vertex> vertices;
			std::vector<unsigned int> indices;

			const auto& meshData = model.mLoDList[0].meshDataList[i];
			if (meshData.isBody)
				meshVec[i].isBody = true;

			vertices.reserve(meshData.meshInfo.vertexCount);
			indices.reserve(meshData.meshInfo.indexCount);

			for (int j = 0; j != meshData.meshInfo.vertexCount; ++j)
			{
				PosColorTexCoord0Vertex v;

				// vertex position
				v.m_x = meshData.vertexData.positions.at(j).x;
				v.m_y = meshData.vertexData.positions.at(j).y;
				v.m_z = meshData.vertexData.positions.at(j).z;

				// vertex color
				if (!meshData.vertexData.colors.empty())
				{
					uint32_t rgba = 0;
					uint8_t r = meshData.vertexData.colors.at(j).r;
					uint8_t g = meshData.vertexData.colors.at(j).g;
					uint8_t b = meshData.vertexData.colors.at(j).b;
					uint8_t a = meshData.vertexData.colors.at(j).a;
					rgba += r;
					rgba <<= 8;
					rgba += g;
					rgba <<= 8;
					rgba += b;
					rgba <<= 8;
					rgba += a;
					v.m_rgba = rgba;
				}
				else
				{
					v.m_rgba = 0x000000FF;
				}

				// texture coordinate
				if (!meshData.vertexData.textureCoordinates0.empty())
				{
					v.m_u = meshData.vertexData.textureCoordinates0.at(j).x;
					v.m_v = meshData.vertexData.textureCoordinates0.at(j).y;
				}
				else
				{
					v.m_u = 0.0f;
					v.m_v = 0.0f;
				}

				// normal
				if (!meshData.vertexData.normals.empty())
				{
					v.m_nx = meshData.vertexData.normals.at(j).x;
					v.m_ny = meshData.vertexData.normals.at(j).y;
					v.m_nz = meshData.vertexData.normals.at(j).z;
				}
				else
				{
					v.m_nx = 0.0f;
					v.m_ny = 0.0f;
					v.m_nz = 0.0f;
				}

				// Tangent & bitangent will be calculated manually while reading indices.
				v.m_tx = 0.0f;
				v.m_ty = 0.0f;
				v.m_tz = 0.0f;
				v.m_bx = 0.0f;
				v.m_by = 0.0f;
				v.m_bz = 0.0f;


				vertices.push_back(v);
			}

			for (int j = 0; j != meshData.meshInfo.indexCount / 3; ++j)
			{
				indices.push_back(meshData.vertexData.indices.at(3 * j));
				indices.push_back(meshData.vertexData.indices.at(3 * j + 2));
				indices.push_back(meshData.vertexData.indices.at(3 * j + 1));
				// IMPORTANT, must be in the reverse clocl-wise order from thr original buffer(0,2,1 instead of 0,1,2) or the model will be front face culled.

				// tangent & bitangent
				PosColorTexCoord0Vertex& p0 = vertices.at(indices.at(3 * j));
				PosColorTexCoord0Vertex& p1 = vertices.at(indices.at(3 * j + 2));
				PosColorTexCoord0Vertex& p2 = vertices.at(indices.at(3 * j + 1));

				calcTanAndBitan(p0, p1, p2);
				calcTanAndBitan(p1, p0, p2);
				calcTanAndBitan(p2, p0, p1);
			}


			meshVec[i].mVertices = std::move(vertices);
			meshVec[i].mIndices = std::move(indices);
		}

		std::cout << "Model Loaded:\n";
		mdl.print();
		return meshVec;
	}

	static Tt::Tex loadTextureThread(const std::wstring& mpdPath,
	                                 unsigned offset, unsigned size, std::wstring_view texType)
	{
		std::ifstream ifs;
		Tt::Tex tex;
		if (size == offset)
		{
			std::wcerr << texType << "Texture doesn't exist" << std::endl;
			return tex;
		}
		if (!tex.readRawFromDisk(ifs, mpdPath, offset, size))
		{
			std::wcerr << "[ERROR] Could not read" << texType << " texture from: \n"
				<< mpdPath << ", offset: " << offset << ", size: " << size << std::endl;
			return tex;
		}
		if (!tex.loadFromRaw())
		{
			std::wcerr << "[ERROR] Could not read " << texType << " texture from: \n"
				<< mpdPath << ", offset: " << offset << ", size: " << size << std::endl;
			return tex;
		}
		std::wcout << texType << "Texture Loaded: \n";
		tex.print();
		return tex;
	}

	void TTMPViewer::loadThread(unsigned mdlOff, unsigned mdlSize,
	                            unsigned diffOff, unsigned diffSize,
	                            unsigned normOff, unsigned normSize,
	                            unsigned specOff, unsigned specSize)
	{
		std::wstring mpdPath = L".\\tmp\\";
		mpdPath += (mCurrentModpackName + L"\\TTMPD.mpd");
		// async version
		auto loadMdlFut = std::async(std::launch::async, loadModelThread, mpdPath, mdlOff, mdlSize);
		auto loadDiffFut = std::async(std::launch::async, loadTextureThread, mpdPath, diffOff, diffSize, L"Diffuse");
		auto loadNormFut = std::async(std::launch::async, loadTextureThread, mpdPath, normOff, normSize, L"Normal");
		auto loadSpecFut = std::async(std::launch::async, loadTextureThread, mpdPath, specOff, specSize,
		                              L"Specular/Metallic");

		std::vector<BgfxMesh> meshVec = std::move(loadMdlFut.get());
		const Tt::Tex diffTex = std::move(loadDiffFut.get());
		const Tt::Tex normTex = std::move(loadNormFut.get());
		const Tt::Tex specTex = std::move(loadSpecFut.get());

		for (auto& mesh : meshVec)
		{
			if (!mesh.isBody)
			{
				mesh.mDiffuse = diffTex;
				mesh.mNormal = normTex;
				mesh.mSpecular = specTex;
			}

			mesh.bindHandles();
		}

		// sync version
		/*std::vector<BgfxMesh> meshVec = std::move(loadModelThread(mpdPath, mdlOff, mdlSize, 1.0));
		for (auto& mesh : meshVec)
		{
			if (!mesh.isBody)
			{
				mesh.mDiffuse = std::move(loadTextureThread(mpdPath, diffOff, diffSize, L"Diffuse"));
				mesh.mNormal = std::move(loadTextureThread(mpdPath, normOff, normSize, L"Normal"));
				mesh.mSpecular = std::move(loadTextureThread(mpdPath, specOff, specSize, L"Specular/Metallic"));
			}

			mesh.bindHandles();
		}*/

		// mRefreshingMeshes = true;
		mRefreshingMeshesMutex.lock();
		for (auto& mesh : mMeshes)
		{
			destroy(mesh.mVbh);
			destroy(mesh.mIbh);
			destroy(mesh.mDth);
			destroy(mesh.mNth);
			destroy(mesh.mSth);
		}
		mMeshes = std::move(meshVec);
		mRefreshingMeshesMutex.unlock();
		// mRefreshingMeshes = false;

		mLoadComplete = true;
	}
} // namespace

int _main_(int _argc, char** _argv)
{
	std::locale::global(std::locale("")); // to normally display non-ascii characters in console.
	TTMPViewer ttmpViewerApp("TTMPPreviewer",
	                         "Modified straightly from:\n BGFX/Example18: IBL",
	                         "https://github.com/bkaradzic/bgfx/tree/master/examples/18-ibl");
	return entry::runApp(&ttmpViewerApp, _argc, _argv);
}
