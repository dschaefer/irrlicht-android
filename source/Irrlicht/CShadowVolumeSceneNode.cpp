// Copyright (C) 2002-2009 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CShadowVolumeSceneNode.h"
#include "ISceneManager.h"
#include "IMesh.h"
#include "IVideoDriver.h"
#include "SLight.h"

namespace irr
{
namespace scene
{


//! constructor
CShadowVolumeSceneNode::CShadowVolumeSceneNode(const IMesh* shadowMesh, ISceneNode* parent,
		ISceneManager* mgr, s32 id, bool zfailmethod, f32 infinity)
: IShadowVolumeSceneNode(parent, mgr, id),
	ShadowMesh(0), IndexCount(0), VertexCount(0), ShadowVolumesUsed(0),
	Infinity(infinity), UseZFailMethod(zfailmethod)
{
	#ifdef _DEBUG
	setDebugName("CShadowVolumeSceneNode");
	#endif
	setShadowMesh(shadowMesh);
	setAutomaticCulling(scene::EAC_OFF);
}


//! destructor
CShadowVolumeSceneNode::~CShadowVolumeSceneNode()
{
	if (ShadowMesh)
		ShadowMesh->drop();

	for (u32 i=0; i<ShadowVolumes.size(); ++i)
		delete [] ShadowVolumes[i].vertices;
}


void CShadowVolumeSceneNode::createShadowVolume(const core::vector3df& light)
{
	SShadowVolume* svp = 0;

	// builds the shadow volume and adds it to the shadow volume list.

	if (ShadowVolumes.size() > ShadowVolumesUsed)
	{
		// get the next unused buffer
		svp = &ShadowVolumes[ShadowVolumesUsed];
		if (svp->size >= IndexCount*5)
			svp->count = 0;
		else
		{
			svp->size = IndexCount*5;
			svp->count = 0;
			delete [] svp->vertices;
			svp->vertices = new core::vector3df[svp->size];
		}

		++ShadowVolumesUsed;
	}
	else
	{
		// add a buffer
		SShadowVolume tmp;
		// lets make a rather large shadowbuffer
		tmp.size = IndexCount*5;
		tmp.count = 0;
		tmp.vertices = new core::vector3df[tmp.size];
		ShadowVolumes.push_back(tmp);
		svp = &ShadowVolumes[ShadowVolumes.size()-1];
		++ShadowVolumesUsed;
	}

	const u32 faceCount = IndexCount / 3;

	if (faceCount * 6 > Edges.size())
		Edges.set_used(faceCount*6);

	u32 numEdges = 0;
	const core::vector3df ls = light * Infinity; // light scaled

	//if (UseZFailMethod)
	//	createZFailVolume(faceCount, numEdges, light, svp);
	//else
	//	createZPassVolume(faceCount, numEdges, light, svp, false);

	// the createZFailVolume does currently not work 100% correctly,
	// so we create createZPassVolume with caps if the zfail method
	// is used
	createZPassVolume(faceCount, numEdges, light, svp, UseZFailMethod);

	for (u32 i=0; i<numEdges; ++i)
	{
		core::vector3df &v1 = Vertices[Edges[2*i+0]];
		core::vector3df &v2 = Vertices[Edges[2*i+1]];
		core::vector3df v3(v1 - ls);
		core::vector3df v4(v2 - ls);

		// Add a quad (two triangles) to the vertex list
		if (svp->vertices && svp->count < svp->size-5)
		{
			svp->vertices[svp->count++] = v1;
			svp->vertices[svp->count++] = v2;
			svp->vertices[svp->count++] = v3;

			svp->vertices[svp->count++] = v2;
			svp->vertices[svp->count++] = v4;
			svp->vertices[svp->count++] = v3;
		}
	}
}


void CShadowVolumeSceneNode::createZFailVolume(s32 faceCount, u32& numEdges,
						const core::vector3df& light,
						SShadowVolume* svp)
{
	s32 i;
	const core::vector3df ls = light * Infinity;

	// Check every face if it is front or back facing the light.
	for (i=0; i<faceCount; ++i)
	{
		const u16 wFace0 = Indices[3*i+0];
		const u16 wFace1 = Indices[3*i+1];
		const u16 wFace2 = Indices[3*i+2];

		const core::vector3df v0 = Vertices[wFace0];
		const core::vector3df v1 = Vertices[wFace1];
		const core::vector3df v2 = Vertices[wFace2];

		if (core::triangle3df(v0,v1,v2).isFrontFacing(light))
		{
			FaceData[i] = false; // it's a back facing face

			if (svp->vertices && svp->count < svp->size-5)
			{
				// add front cap
				svp->vertices[svp->count++] = v0;
				svp->vertices[svp->count++] = v2;
				svp->vertices[svp->count++] = v1;

				// add back cap
				svp->vertices[svp->count++] = v0 - ls;
				svp->vertices[svp->count++] = v1 - ls;
				svp->vertices[svp->count++] = v2 - ls;
			}
		}
		else
			FaceData[i] = true; // it's a front facing face
	}

	for(i=0; i<faceCount; ++i)
	{
		if (FaceData[i] == true)
		{
			const u16 wFace0 = Indices[3*i+0];
			const u16 wFace1 = Indices[3*i+1];
			const u16 wFace2 = Indices[3*i+2];

			const u16 adj0 = Adjacency[3*i+0];
			const u16 adj1 = Adjacency[3*i+1];
			const u16 adj2 = Adjacency[3*i+2];

			if (adj0 != (u16)-1 && FaceData[adj0] == false)
			{
				// add edge v0-v1
				Edges[2*numEdges+0] = wFace0;
				Edges[2*numEdges+1] = wFace1;
				++numEdges;
			}

			if (adj1 != (u16)-1 && FaceData[adj1] == false)
			{
				// add edge v1-v2
				Edges[2*numEdges+0] = wFace1;
				Edges[2*numEdges+1] = wFace2;
				++numEdges;
			}

			if (adj2 != (u16)-1 && FaceData[adj2] == false)
			{
				// add edge v2-v0
				Edges[2*numEdges+0] = wFace2;
				Edges[2*numEdges+1] = wFace0;
				++numEdges;
			}
		}
	}
}


void CShadowVolumeSceneNode::createZPassVolume(s32 faceCount,
						u32& numEdges,
						core::vector3df light,
						SShadowVolume* svp, bool caps)
{
	light *= Infinity;
	if (light == core::vector3df(0,0,0))
		light = core::vector3df(0.0001f,0.0001f,0.0001f);

	for (s32 i=0; i<faceCount; ++i)
	{
		const u16 wFace0 = Indices[3*i+0];
		const u16 wFace1 = Indices[3*i+1];
		const u16 wFace2 = Indices[3*i+2];

		if (core::triangle3df(Vertices[wFace0],Vertices[wFace1],Vertices[wFace2]).isFrontFacing(light))
		{
			Edges[2*numEdges+0] = wFace0;
			Edges[2*numEdges+1] = wFace1;
			++numEdges;

			Edges[2*numEdges+0] = wFace1;
			Edges[2*numEdges+1] = wFace2;
			++numEdges;

			Edges[2*numEdges+0] = wFace2;
			Edges[2*numEdges+1] = wFace0;
			++numEdges;

			if (caps && svp->vertices && svp->count < svp->size-5)
			{
				svp->vertices[svp->count++] = Vertices[wFace0];
				svp->vertices[svp->count++] = Vertices[wFace2];
				svp->vertices[svp->count++] = Vertices[wFace1];

				svp->vertices[svp->count++] = Vertices[wFace0] - light;
				svp->vertices[svp->count++] = Vertices[wFace1] - light;
				svp->vertices[svp->count++] = Vertices[wFace2] - light;
			}
		}
	}
}


void CShadowVolumeSceneNode::setShadowMesh(const IMesh* mesh)
{
    if ( ShadowMesh == mesh )
        return;
	if (ShadowMesh)
		ShadowMesh->drop();
	ShadowMesh = mesh;
	if (ShadowMesh)
		ShadowMesh->grab();
}


void CShadowVolumeSceneNode::updateShadowVolumes()
{
	const u32 oldIndexCount = IndexCount;
	const u32 oldVertexCount = VertexCount;

	VertexCount = 0;
	IndexCount = 0;
	ShadowVolumesUsed = 0;

	const IMesh* const mesh = ShadowMesh;
	if (!mesh)
		return;

	// calculate total amount of vertices and indices

	u32 i;
	u32 totalVertices = 0;
	u32 totalIndices = 0;
	const u32 bufcnt = mesh->getMeshBufferCount();

	for (i=0; i<bufcnt; ++i)
	{
		const IMeshBuffer* buf = mesh->getMeshBuffer(i);
		totalIndices += buf->getIndexCount();
		totalVertices += buf->getVertexCount();
	}

	// allocate memory if necessary

	if (totalVertices > Vertices.size())
		Vertices.set_used(totalVertices);

	if (totalIndices > Indices.size())
	{
		Indices.set_used(totalIndices);

		if (UseZFailMethod)
			FaceData.set_used(totalIndices / 3);
	}

	// copy mesh

	for (i=0; i<bufcnt; ++i)
	{
		const IMeshBuffer* buf = mesh->getMeshBuffer(i);

		const u16* idxp = buf->getIndices();
		const u16* idxpend = idxp + buf->getIndexCount();
		for (; idxp!=idxpend; ++idxp)
			Indices[IndexCount++] = *idxp + VertexCount;

		const u32 vtxcnt = buf->getVertexCount();
		for (u32 j=0; j<vtxcnt; ++j)
			Vertices[VertexCount++] = buf->getPosition(j);
	}

	// recalculate adjacency if necessary
	if (oldVertexCount != VertexCount && oldIndexCount != IndexCount && UseZFailMethod)
		calculateAdjacency();

	// create as much shadow volumes as there are lights but
	// do not ignore the max light settings.

	const u32 lights = SceneManager->getVideoDriver()->getDynamicLightCount();
	core::matrix4 mat = Parent->getAbsoluteTransformation();
	mat.makeInverse();
	const core::vector3df parentpos = Parent->getAbsolutePosition();
	core::vector3df lpos;

	// TODO: Only correct for point lights.
	for (i=0; i<lights; ++i)
	{
		const video::SLight& dl = SceneManager->getVideoDriver()->getDynamicLight(i);
		lpos = dl.Position;
		if (dl.CastShadows &&
			fabs((lpos - parentpos).getLengthSQ()) <= (dl.Radius*dl.Radius*4.0f))
		{
			mat.transformVect(lpos);
			createShadowVolume(lpos);
		}
	}
}


//! pre render method
void CShadowVolumeSceneNode::OnRegisterSceneNode()
{
	if (IsVisible)
	{
		SceneManager->registerNodeForRendering(this, scene::ESNRP_SHADOW);
		ISceneNode::OnRegisterSceneNode();
	}
}


//! renders the node.
void CShadowVolumeSceneNode::render()
{
	video::IVideoDriver* driver = SceneManager->getVideoDriver();

	if (!ShadowVolumesUsed || !driver)
		return;

	driver->setTransform(video::ETS_WORLD, Parent->getAbsoluteTransformation());

	for (u32 i=0; i<ShadowVolumesUsed; ++i)
		driver->drawStencilShadowVolume(ShadowVolumes[i].vertices,
			ShadowVolumes[i].count, UseZFailMethod);

	if ( DebugDataVisible & scene::EDS_MESH_WIRE_OVERLAY )
	{
		video::SMaterial mat;
		mat.Lighting = false;
		mat.Wireframe = true;
		mat.ZBuffer = true;
		driver->setMaterial(mat);
		driver->setTransform(video::ETS_WORLD, core::IdentityMatrix);

		for (u32 i=0; i<ShadowVolumesUsed; ++i)
			driver->drawVertexPrimitiveList(ShadowVolumes[i].vertices,
			ShadowVolumes[i].count,0,0,video::EVT_STANDARD,scene::EPT_LINES);
	}
}


//! returns the axis aligned bounding box of this node
const core::aabbox3d<f32>& CShadowVolumeSceneNode::getBoundingBox() const
{
	return Box;
}


//! Generates adjacency information based on mesh indices.
void CShadowVolumeSceneNode::calculateAdjacency(f32 epsilon)
{
	Adjacency.set_used(IndexCount);

	epsilon *= epsilon;

	// go through all faces and fetch their three neighbours
	for (u32 f=0; f<IndexCount; f+=3)
	{
		for (u32 edge = 0; edge<3; ++edge)
		{
			core::vector3df v1 = Vertices[Indices[f+edge]];
			core::vector3df v2 = Vertices[Indices[f+((edge+1)%3)]];

			// now we search an_O_ther _F_ace with these two
			// vertices, which is not the current face.

			u32 of;

			for (of=0; of<IndexCount; of+=3)
			{
				if (of != f)
				{
					s32 cnt1 = 0;
					s32 cnt2 = 0;

					for (s32 e=0; e<3; ++e)
					{
						const f32 t1 = v1.getDistanceFromSQ(Vertices[Indices[of+e]]);
						if (core::iszero(t1))
							++cnt1;

						const f32 t2 = v2.getDistanceFromSQ(Vertices[Indices[of+e]]);
						if (core::iszero(t2))
							++cnt2;
					}

					if (cnt1 == 1 && cnt2 == 1)
						break;
				}
			}

			if (of == IndexCount)
				Adjacency[f + edge] = f;
			else
				Adjacency[f + edge] = of / 3;
		}
	}
}


} // end namespace scene
} // end namespace irr

