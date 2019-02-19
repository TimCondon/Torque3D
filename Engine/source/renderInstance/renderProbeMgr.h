//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------
#pragma once
#ifndef RENDER_PROBE_MGR_H
#define RENDER_PROBE_MGR_H

#ifndef _RENDERBINMANAGER_H_
#include "renderInstance/renderBinManager.h"
#endif
#ifndef _MATINSTANCE_H_
#include "materials/matInstance.h"
#endif
#ifndef _MATTEXTURETARGET_H_
#include "materials/matTextureTarget.h"
#endif
#ifndef _GFXPRIMITIVEBUFFER_H_
#include "gfx/gfxPrimitiveBuffer.h"
#endif
#ifndef _GFXVERTEXBUFFER_H_
#include "gfx/gfxVertexBuffer.h"
#endif

#include "core/util/SystemInterfaceList.h"

#ifndef _MATERIALS_PROCESSEDSHADERMATERIAL_H_
#include "materials/processedShaderMaterial.h"
#endif
#ifndef _POSTEFFECTCOMMON_H_
#include "postFx/postEffectCommon.h"
#endif

#ifndef _REFLECTOR_H_
#include "scene/reflector.h"
#endif

static U32 MAXPROBECOUNT = 50;

class PostEffect;
class ReflectionProbe;

struct ProbeRenderInst : public SystemInterface<ProbeRenderInst>
{
   MatrixF mTransform;

   F32 mRadius;

   bool mDirty;

   Box3F mBounds;
   Point3F mPosition;
   Point3F mProbeRefOffset;
   Point3F mProbeRefScale;

   GFXCubemapHandle mCubemap;
   GFXCubemapHandle mIrradianceCubemap;

   //Utilized in dynamic reflections
   CubeReflector mCubeReflector;

   /// The priority of this light used for
   /// light and shadow scoring.
   F32 mPriority;

   /// A temporary which holds the score used
   /// when prioritizing lights for rendering.
   F32 mScore;

   bool mIsSkylight;

   enum ProbeShapeType
   {
      Sphere = 0,            ///< Sphere shaped
      Box = 1,               ///< Box-based shape
   };

   ProbeShapeType mProbeShapeType;

public:

   ProbeRenderInst();
   ~ProbeRenderInst();

   // Copies data passed in from light
   void set(const ProbeRenderInst *probeInfo);

   // Accessors
   const MatrixF& getTransform() const { return mTransform; }
   void setTransform(const MatrixF &xfm) { mTransform = xfm; }

   Point3F getPosition() const { return mPosition; }
   void setPosition(const Point3F &pos) { mPosition = pos; }

   VectorF getDirection() const { return mTransform.getForwardVector(); }
   void setDirection(const VectorF &val);

   void setPriority(F32 priority) { mPriority = priority; }
   F32 getPriority() const { return mPriority; }

   void setScore(F32 score) { mScore = score; }
   F32 getScore() const { return mScore; }

   void clear();
};

struct ProbeShaderConstants
{
   bool mInit;

   GFXShaderRef mShader;

   GFXShaderConstHandle* mProbeParamsSC;

   //Reflection Probes
   GFXShaderConstHandle *mProbePositionSC;
   GFXShaderConstHandle *mProbeRadiusSC;
   GFXShaderConstHandle *mProbeBoxMinSC;
   GFXShaderConstHandle *mProbeBoxMaxSC;
   GFXShaderConstHandle *mProbeIsSphereSC;
   GFXShaderConstHandle *mProbeLocalPosSC;
   GFXShaderConstHandle *mProbeCubemapSC;
   GFXShaderConstHandle *mProbeCountSC;

   ProbeShaderConstants();
   ~ProbeShaderConstants();

   void init(GFXShader* buffer);

   void _onShaderReload();
};

typedef Map<GFXShader*, ProbeShaderConstants*> ProbeConstantMap;

//**************************************************************************
// RenderObjectMgr
//**************************************************************************
class RenderProbeMgr : public RenderBinManager
{
   typedef RenderBinManager Parent;

   Vector<U32> mRegisteredProbes;

   //Array rendering
   U32 mEffectiveProbeCount;
   S32 mMipCount;
   Vector<Point4F> probePositionsData;
   Vector<Point4F> probeRefPositionsData;
   Vector<MatrixF> probeWorldToObjData;
   Vector<Point4F> probeBBMinData;
   Vector<Point4F> probeBBMaxData;
   Vector<Point4F> probeConfigData;
   Vector<GFXCubemapHandle> cubeMaps;
   Vector<GFXCubemapHandle> irradMaps;

   AlignedArray<Point4F> mProbePositions;
   AlignedArray<Point4F> mProbeBBMin;
   AlignedArray<Point4F> mProbeBBMax;
   AlignedArray<float> mProbeUseSphereMode;
   AlignedArray<float> mProbeRadius;
   AlignedArray<float> mProbeAttenuation;

   GFXCubemapArrayHandle mCubemapArray;
   GFXCubemapArrayHandle mIrradArray;

   //Utilized in forward rendering
   ProbeConstantMap mConstantLookup;
   GFXShaderRef mLastShader;
   ProbeShaderConstants* mLastConstants;

   //
   SimObjectPtr<PostEffect> mProbeArrayEffect;

public:
   RenderProbeMgr();
   RenderProbeMgr(RenderInstType riType, F32 renderOrder, F32 processAddOrder);

   virtual void onRemove();

   // ConsoleObject
   static void initPersistFields();
   DECLARE_CONOBJECT(RenderProbeMgr);

protected:
   /// The current active light manager.
   static RenderProbeMgr *smProbeManager;

   /// This helper function sets the shader constansts
   /// for the stock 4 light forward lighting code.
   void _update4ProbeConsts(const SceneData &sgData,
      MatrixSet &matSet,
      GFXShaderConstHandle *probePositionSC,
      GFXShaderConstHandle *probeRadiusSC,
      GFXShaderConstHandle *probeBoxMinSC,
      GFXShaderConstHandle *probeBoxMaxSC,
      GFXShaderConstHandle *probeCubemapSC,
      GFXShaderConstHandle *probeIsSphereSC,
      GFXShaderConstHandle *probeLocalPosSC,
      GFXShaderConstBuffer *shaderConsts);

   void _setupStaticParameters();
   void _setupPerFrameParameters(const SceneRenderState *state);
   virtual void addElement(RenderInst *inst);
   virtual void render(SceneRenderState * state);

   ProbeShaderConstants* getProbeShaderConstants(GFXShaderConstBuffer* buffer);

   PostEffect* getProbeArrayEffect();

public:
   // RenderBinMgr
   void updateProbes();

   /// Returns the active LM.
   static inline RenderProbeMgr* getProbeManager();

   void registerProbe(U32 probeIdx);
   void unregisterProbe(U32 probeIdx);

   virtual void setProbeInfo(ProcessedMaterial *pmat,
	   const Material *mat,
	   const SceneData &sgData,
	   const SceneRenderState *state,
	   U32 pass,
	   GFXShaderConstBuffer *shaderConsts);

   /// Debug rendering
   static bool smRenderReflectionProbes;

   void bakeProbe(ReflectionProbe *probeInfo);
   void bakeProbes();
};

RenderProbeMgr* RenderProbeMgr::getProbeManager()
{
   if (smProbeManager == nullptr)
   {
      RenderProbeMgr* probeManager = new RenderProbeMgr();

      smProbeManager = probeManager;
   }

   return smProbeManager;
}

#define PROBEMGR RenderProbeMgr::getProbeManager()

#endif // RENDER_PROBE_MGR_H