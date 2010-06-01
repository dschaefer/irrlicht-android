// Copyright (C) 2002-2009 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CMeshManipulator.h"
#include "SMesh.h"
#include "CMeshBuffer.h"
#include "SAnimatedMesh.h"
#include "os.h"
#include "irrMap.h"

namespace irr
{
namespace scene
{

static inline core::vector3df getAngleWeight(const core::vector3df& v1,
		const core::vector3df& v2,
		const core::vector3df& v3)
{
	// Calculate this triangle's weight for each of its three vertices
	// start by calculating the lengths of its sides
	const f32 a = v2.getDistanceFromSQ(v3);
	const f32 asqrt = sqrtf(a);
	const f32 b = v1.getDistanceFromSQ(v3);
	const f32 bsqrt = sqrtf(b);
	const f32 c = v1.getDistanceFromSQ(v2);
	const f32 csqrt = sqrtf(c);

	// use them to find the angle at each vertex
	return core::vector3df(
		acosf((b + c - a) / (2.f * bsqrt * csqrt)),
		acosf((-b + c + a) / (2.f * asqrt * csqrt)),
		acosf((b - c + a) / (2.f * bsqrt * asqrt)));
}


//! Flips the direction of surfaces. Changes backfacing triangles to frontfacing
//! triangles and vice versa.
//! \param mesh: Mesh on which the operation is performed.
void CMeshManipulator::flipSurfaces(scene::IMesh* mesh) const
{
	if (!mesh)
		return;

	const u32 bcount = mesh->getMeshBufferCount();
	for (u32 b=0; b<bcount; ++b)
	{
		IMeshBuffer* buffer = mesh->getMeshBuffer(b);
		const u32 idxcnt = buffer->getIndexCount();
		u16* idx = buffer->getIndices();
		s32 tmp;

		for (u32 i=0; i<idxcnt; i+=3)
		{
			tmp = idx[i+1];
			idx[i+1] = idx[i+2];
			idx[i+2] = tmp;
		}
	}
}


//! Recalculates all normals of the mesh buffer.
/** \param buffer: Mesh buffer on which the operation is performed. */
void CMeshManipulator::recalculateNormals(IMeshBuffer* buffer, bool smooth, bool angleWeighted) const
{
	if (!buffer)
		return;

	const u32 vtxcnt = buffer->getVertexCount();
	const u32 idxcnt = buffer->getIndexCount();
	const u16* idx = buffer->getIndices();

	if (!smooth)
	{
		for (u32 i=0; i<idxcnt; i+=3)
		{
			const core::vector3df& v1 = buffer->getPosition(idx[i+0]);
			const core::vector3df& v2 = buffer->getPosition(idx[i+1]);
			const core::vector3df& v3 = buffer->getPosition(idx[i+2]);
			const core::vector3df normal = core::plane3d<f32>(v1, v2, v3).Normal;
			buffer->getNormal(idx[i+0]) = normal;
			buffer->getNormal(idx[i+1]) = normal;
			buffer->getNormal(idx[i+2]) = normal;
		}
	}
	else
	{
		u32 i;

		for ( i = 0; i!= vtxcnt; ++i )
			buffer->getNormal(i).set( 0.f, 0.f, 0.f );

		for ( i=0; i<idxcnt; i+=3)
		{
			const core::vector3df& v1 = buffer->getPosition(idx[i+0]);
			const core::vector3df& v2 = buffer->getPosition(idx[i+1]);
			const core::vector3df& v3 = buffer->getPosition(idx[i+2]);
			core::vector3df normal = core::plane3d<f32>(v1, v2, v3).Normal;

			if (angleWeighted)
				normal *= getAngleWeight(v1,v2,v3);

			buffer->getNormal(idx[i+0]) += normal;
			buffer->getNormal(idx[i+1]) += normal;
			buffer->getNormal(idx[i+2]) += normal;
		}

		for ( i = 0; i!= vtxcnt; ++i )
			buffer->getNormal(i).normalize();
	}
}


//! Recalculates all normals of the mesh.
//! \param mesh: Mesh on which the operation is performed.
void CMeshManipulator::recalculateNormals(scene::IMesh* mesh, bool smooth, bool angleWeighted) const
{
	if (!mesh)
		return;

	const u32 bcount = mesh->getMeshBufferCount();
	for ( u32 b=0; b<bcount; ++b)
		recalculateNormals(mesh->getMeshBuffer(b), smooth, angleWeighted);
}


//! Recalculates tangents, requires a tangent mesh
void CMeshManipulator::recalculateTangents(IMesh* mesh, bool recalculateNormals, bool smooth, bool angleWeighted) const
{
	if (!mesh || !mesh->getMeshBufferCount() || (mesh->getMeshBuffer(0)->getVertexType()!= video::EVT_TANGENTS))
		return;

	const u32 meshBufferCount = mesh->getMeshBufferCount();
	for (u32 b=0; b<meshBufferCount; ++b)
	{
		IMeshBuffer* clone = mesh->getMeshBuffer(b);
		const u32 vtxCnt = clone->getVertexCount();
		const u32 idxCnt = clone->getIndexCount();

		u16* idx = clone->getIndices();
		video::S3DVertexTangents* v =
			(video::S3DVertexTangents*)clone->getVertices();

		if (smooth)
		{
			u32 i;

			for ( i = 0; i!= vtxCnt; ++i )
			{
				if (recalculateNormals)
					v[i].Normal.set( 0.f, 0.f, 0.f );
				v[i].Tangent.set( 0.f, 0.f, 0.f );
				v[i].Binormal.set( 0.f, 0.f, 0.f );
			}

			//Each vertex gets the sum of the tangents and binormals from the faces around it
			for ( i=0; i<idxCnt; i+=3)
			{
				// if this triangle is degenerate, skip it!
				if (v[idx[i+0]].Pos == v[idx[i+1]].Pos || 
					v[idx[i+0]].Pos == v[idx[i+2]].Pos || 
					v[idx[i+1]].Pos == v[idx[i+2]].Pos 
					/*||
					v[idx[i+0]].TCoords == v[idx[i+1]].TCoords || 
					v[idx[i+0]].TCoords == v[idx[i+2]].TCoords || 
					v[idx[i+1]].TCoords == v[idx[i+2]].TCoords */
					) 
					continue;

				//Angle-weighted normals look better, but are slightly more CPU intensive to calculate
				core::vector3df weight(1.f,1.f,1.f);
				if (angleWeighted)
					weight = getAngleWeight(v[i+0].Pos,v[i+1].Pos,v[i+2].Pos);
				core::vector3df localNormal; 
				core::vector3df localTangent;
				core::vector3df localBinormal;

				calculateTangents(
					localNormal,
					localTangent,
					localBinormal,
					v[idx[i+0]].Pos,
					v[idx[i+1]].Pos,
					v[idx[i+2]].Pos,
					v[idx[i+0]].TCoords,
					v[idx[i+1]].TCoords,
					v[idx[i+2]].TCoords);

				if (recalculateNormals)
					v[idx[i+0]].Normal += localNormal * weight.X;
				v[idx[i+0]].Tangent += localTangent * weight.X;
				v[idx[i+0]].Binormal += localBinormal * weight.X;
				
				calculateTangents(
					localNormal,
					localTangent,
					localBinormal,
					v[idx[i+1]].Pos,
					v[idx[i+2]].Pos,
					v[idx[i+0]].Pos,
					v[idx[i+1]].TCoords,
					v[idx[i+2]].TCoords,
					v[idx[i+0]].TCoords);

				if (recalculateNormals)
					v[idx[i+1]].Normal += localNormal * weight.Y;
				v[idx[i+1]].Tangent += localTangent * weight.Y;
				v[idx[i+1]].Binormal += localBinormal * weight.Y;

				calculateTangents(
					localNormal,
					localTangent,
					localBinormal,
					v[idx[i+2]].Pos,
					v[idx[i+0]].Pos,
					v[idx[i+1]].Pos,
					v[idx[i+2]].TCoords,
					v[idx[i+0]].TCoords,
					v[idx[i+1]].TCoords);

				if (recalculateNormals)
					v[idx[i+2]].Normal += localNormal * weight.Z;
				v[idx[i+2]].Tangent += localTangent * weight.Z;
				v[idx[i+2]].Binormal += localBinormal * weight.Z;
			}

			// Normalize the tangents and binormals
			if (recalculateNormals)
			{
				for ( i = 0; i!= vtxCnt; ++i )
					v[i].Normal.normalize();
			}
			for ( i = 0; i!= vtxCnt; ++i )
			{
				v[i].Tangent.normalize();
				v[i].Binormal.normalize();
			}
		}
		else
		{
			core::vector3df localNormal; 
			for (u32 i=0; i<idxCnt; i+=3)
			{
				calculateTangents(
					localNormal,
					v[idx[i+0]].Tangent,
					v[idx[i+0]].Binormal,
					v[idx[i+0]].Pos,
					v[idx[i+1]].Pos,
					v[idx[i+2]].Pos,
					v[idx[i+0]].TCoords,
					v[idx[i+1]].TCoords,
					v[idx[i+2]].TCoords);
				if (recalculateNormals)
					v[idx[i+0]].Normal=localNormal;

				calculateTangents(
					localNormal,
					v[idx[i+1]].Tangent,
					v[idx[i+1]].Binormal,
					v[idx[i+1]].Pos,
					v[idx[i+2]].Pos,
					v[idx[i+0]].Pos,
					v[idx[i+1]].TCoords,
					v[idx[i+2]].TCoords,
					v[idx[i+0]].TCoords);
				if (recalculateNormals)
					v[idx[i+1]].Normal=localNormal;

				calculateTangents(
					localNormal,
					v[idx[i+2]].Tangent,
					v[idx[i+2]].Binormal,
					v[idx[i+2]].Pos,
					v[idx[i+0]].Pos,
					v[idx[i+1]].Pos,
					v[idx[i+2]].TCoords,
					v[idx[i+0]].TCoords,
					v[idx[i+1]].TCoords);
				if (recalculateNormals)
					v[idx[i+2]].Normal=localNormal;
			}
		}
	}
}


//! Clones a static IMesh into a modifyable SMesh.
SMesh* CMeshManipulator::createMeshCopy(scene::IMesh* mesh) const
{
	if (!mesh)
		return 0;

	SMesh* clone = new SMesh();

	const u32 meshBufferCount = mesh->getMeshBufferCount();

	for ( u32 b=0; b<meshBufferCount; ++b)
	{
		switch(mesh->getMeshBuffer(b)->getVertexType())
		{
		case video::EVT_STANDARD:
			{
				SMeshBuffer* buffer = new SMeshBuffer();
				const u32 vcount = mesh->getMeshBuffer(b)->getVertexCount();
				buffer->Vertices.reallocate(vcount);
				video::S3DVertex* vertices = (video::S3DVertex*)mesh->getMeshBuffer(b)->getVertices();
				for (u32 i=0; i < vcount; ++i)
					buffer->Vertices.push_back(vertices[i]);
				const u32 icount = mesh->getMeshBuffer(b)->getIndexCount();
				buffer->Indices.reallocate(icount);
				u16* indices = mesh->getMeshBuffer(b)->getIndices();
				for (u32 i=0; i < icount; ++i)
					buffer->Indices.push_back(indices[i]);
				clone->addMeshBuffer(buffer);
				buffer->drop();
			}
			break;
		case video::EVT_2TCOORDS:
			{
				SMeshBufferLightMap* buffer = new SMeshBufferLightMap();
				const u32 vcount = mesh->getMeshBuffer(b)->getVertexCount();
				buffer->Vertices.reallocate(vcount);
				video::S3DVertex2TCoords* vertices = (video::S3DVertex2TCoords*)mesh->getMeshBuffer(b)->getVertices();
				for (u32 i=0; i < vcount; ++i)
					buffer->Vertices.push_back(vertices[i]);
				const u32 icount = mesh->getMeshBuffer(b)->getIndexCount();
				buffer->Indices.reallocate(icount);
				u16* indices = mesh->getMeshBuffer(b)->getIndices();
				for (u32 i=0; i < icount; ++i)
					buffer->Indices.push_back(indices[i]);
				clone->addMeshBuffer(buffer);
				buffer->drop();
			}
			break;
		case video::EVT_TANGENTS:
			{
				SMeshBufferTangents* buffer = new SMeshBufferTangents();
				const u32 vcount = mesh->getMeshBuffer(b)->getVertexCount();
				buffer->Vertices.reallocate(vcount);
				video::S3DVertexTangents* vertices = (video::S3DVertexTangents*)mesh->getMeshBuffer(b)->getVertices();
				for (u32 i=0; i < vcount; ++i)
					buffer->Vertices.push_back(vertices[i]);
				const u32 icount = mesh->getMeshBuffer(b)->getIndexCount();
				buffer->Indices.reallocate(icount);
				u16* indices = mesh->getMeshBuffer(b)->getIndices();
				for (u32 i=0; i < icount; ++i)
					buffer->Indices.push_back(indices[i]);
				clone->addMeshBuffer(buffer);
				buffer->drop();
			}
			break;
		}// end switch

	}// end for all mesh buffers

	clone->BoundingBox = mesh->getBoundingBox();
	return clone;
}


//! Creates a planar texture mapping on the mesh
void CMeshManipulator::makePlanarTextureMapping(scene::IMesh* mesh, f32 resolution=0.01f) const
{
	if (!mesh)
		return;

	const u32 bcount = mesh->getMeshBufferCount();
	for ( u32 b=0; b<bcount; ++b)
	{
		makePlanarTextureMapping(mesh->getMeshBuffer(b), resolution);
	}
}


//! Creates a planar texture mapping on the meshbuffer
void CMeshManipulator::makePlanarTextureMapping(scene::IMeshBuffer* buffer, f32 resolution) const
{
	u32 idxcnt = buffer->getIndexCount();
	u16* idx = buffer->getIndices();

	for (u32 i=0; i<idxcnt; i+=3)
	{
		core::plane3df p(buffer->getPosition(idx[i+0]), buffer->getPosition(idx[i+1]), buffer->getPosition(idx[i+2]));
		p.Normal.X = fabsf(p.Normal.X);
		p.Normal.Y = fabsf(p.Normal.Y);
		p.Normal.Z = fabsf(p.Normal.Z);
		// calculate planar mapping worldspace coordinates

		if (p.Normal.X > p.Normal.Y && p.Normal.X > p.Normal.Z)
		{
			for (u32 o=0; o!=3; ++o)
			{
				buffer->getTCoords(idx[i+o]).X = buffer->getPosition(idx[i+o]).Y * resolution;
				buffer->getTCoords(idx[i+o]).Y = buffer->getPosition(idx[i+o]).Z * resolution;
			}
		}
		else
		if (p.Normal.Y > p.Normal.X && p.Normal.Y > p.Normal.Z)
		{
			for (u32 o=0; o!=3; ++o)
			{
				buffer->getTCoords(idx[i+o]).X = buffer->getPosition(idx[i+o]).X * resolution;
				buffer->getTCoords(idx[i+o]).Y = buffer->getPosition(idx[i+o]).Z * resolution;
			}
		}
		else
		{
			for (u32 o=0; o!=3; ++o)
			{
				buffer->getTCoords(idx[i+o]).X = buffer->getPosition(idx[i+o]).X * resolution;
				buffer->getTCoords(idx[i+o]).Y = buffer->getPosition(idx[i+o]).Y * resolution;
			}
		}
	}
}


//! Creates a planar texture mapping on the meshbuffer
void CMeshManipulator::makePlanarTextureMapping(scene::IMeshBuffer* buffer, f32 resolutionS, f32 resolutionT, u8 axis, const core::vector3df& offset) const
{
	u32 idxcnt = buffer->getIndexCount();
	u16* idx = buffer->getIndices();

	for (u32 i=0; i<idxcnt; i+=3)
	{
		// calculate planar mapping worldspace coordinates
		if (axis==0)
		{
			for (u32 o=0; o!=3; ++o)
			{
				buffer->getTCoords(idx[i+o]).X = 0.5f+(buffer->getPosition(idx[i+o]).Z + offset.Z) * resolutionS;
				buffer->getTCoords(idx[i+o]).Y = 0.5f-(buffer->getPosition(idx[i+o]).Y + offset.Y) * resolutionT;
			}
		}
		else if (axis==1)
		{
			for (u32 o=0; o!=3; ++o)
			{
				buffer->getTCoords(idx[i+o]).X = 0.5f+(buffer->getPosition(idx[i+o]).X + offset.X) * resolutionS;
				buffer->getTCoords(idx[i+o]).Y = 1.f-(buffer->getPosition(idx[i+o]).Z + offset.Z) * resolutionT;
			}
		}
		else if (axis==2)
		{
			for (u32 o=0; o!=3; ++o)
			{
				buffer->getTCoords(idx[i+o]).X = 0.5f+(buffer->getPosition(idx[i+o]).X + offset.X) * resolutionS;
				buffer->getTCoords(idx[i+o]).Y = 0.5f-(buffer->getPosition(idx[i+o]).Y + offset.Y) * resolutionT;
			}
		}
	}
}


//! Creates a copy of the mesh, which will only consist of unique primitives
IMesh* CMeshManipulator::createMeshUniquePrimitives(IMesh* mesh) const
{
	if (!mesh)
		return 0;

	SMesh* clone = new SMesh();

	const u32 meshBufferCount = mesh->getMeshBufferCount();

	for ( u32 b=0; b<meshBufferCount; ++b)
	{
		const s32 idxCnt = mesh->getMeshBuffer(b)->getIndexCount();
		const u16* idx = mesh->getMeshBuffer(b)->getIndices();

		switch(mesh->getMeshBuffer(b)->getVertexType())
		{
		case video::EVT_STANDARD:
			{
				SMeshBuffer* buffer = new SMeshBuffer();
				buffer->Material = mesh->getMeshBuffer(b)->getMaterial();

				video::S3DVertex* v =
					(video::S3DVertex*)mesh->getMeshBuffer(b)->getVertices();

				buffer->Vertices.reallocate(idxCnt);
				buffer->Indices.reallocate(idxCnt);
				for (s32 i=0; i<idxCnt; i += 3)
				{
					buffer->Vertices.push_back( v[idx[i + 0 ]] );
					buffer->Vertices.push_back( v[idx[i + 1 ]] );
					buffer->Vertices.push_back( v[idx[i + 2 ]] );

					buffer->Indices.push_back( i + 0 );
					buffer->Indices.push_back( i + 1 );
					buffer->Indices.push_back( i + 2 );
				}

				buffer->setBoundingBox(mesh->getMeshBuffer(b)->getBoundingBox());
				clone->addMeshBuffer(buffer);
				buffer->drop();
			}
			break;
		case video::EVT_2TCOORDS:
			{
				SMeshBufferLightMap* buffer = new SMeshBufferLightMap();
				buffer->Material = mesh->getMeshBuffer(b)->getMaterial();

				video::S3DVertex2TCoords* v =
					(video::S3DVertex2TCoords*)mesh->getMeshBuffer(b)->getVertices();

				buffer->Vertices.reallocate(idxCnt);
				buffer->Indices.reallocate(idxCnt);
				for (s32 i=0; i<idxCnt; i += 3)
				{
					buffer->Vertices.push_back( v[idx[i + 0 ]] );
					buffer->Vertices.push_back( v[idx[i + 1 ]] );
					buffer->Vertices.push_back( v[idx[i + 2 ]] );

					buffer->Indices.push_back( i + 0 );
					buffer->Indices.push_back( i + 1 );
					buffer->Indices.push_back( i + 2 );
				}
				buffer->setBoundingBox(mesh->getMeshBuffer(b)->getBoundingBox());
				clone->addMeshBuffer(buffer);
				buffer->drop();
			}
			break;
		case video::EVT_TANGENTS:
			{
				SMeshBufferTangents* buffer = new SMeshBufferTangents();
				buffer->Material = mesh->getMeshBuffer(b)->getMaterial();

				video::S3DVertexTangents* v =
					(video::S3DVertexTangents*)mesh->getMeshBuffer(b)->getVertices();

				buffer->Vertices.reallocate(idxCnt);
				buffer->Indices.reallocate(idxCnt);
				for (s32 i=0; i<idxCnt; i += 3)
				{
					buffer->Vertices.push_back( v[idx[i + 0 ]] );
					buffer->Vertices.push_back( v[idx[i + 1 ]] );
					buffer->Vertices.push_back( v[idx[i + 2 ]] );

					buffer->Indices.push_back( i + 0 );
					buffer->Indices.push_back( i + 1 );
					buffer->Indices.push_back( i + 2 );
				}

				buffer->setBoundingBox(mesh->getMeshBuffer(b)->getBoundingBox());
				clone->addMeshBuffer(buffer);
				buffer->drop();
			}
			break;
		}// end switch

	}// end for all mesh buffers

	clone->BoundingBox = mesh->getBoundingBox();
	return clone;
}

//! Creates a copy of a mesh, which will have identical vertices welded together
IMesh* CMeshManipulator::createMeshWelded(IMesh *mesh, f32 tolerance) const
{
	SMesh* clone = new SMesh();
	clone->BoundingBox = mesh->getBoundingBox();

	core::array<u16> redirects;

	for (u32 b=0; b<mesh->getMeshBufferCount(); ++b)
	{
		// reset redirect list
		redirects.set_used(mesh->getMeshBuffer(b)->getVertexCount());

		u16* indices = 0;
		u32 indexCount = 0;
		core::array<u16>* outIdx = 0;

		switch(mesh->getMeshBuffer(b)->getVertexType())
		{
		case video::EVT_STANDARD:
		{
			SMeshBuffer* buffer = new SMeshBuffer();
			buffer->BoundingBox = mesh->getMeshBuffer(b)->getBoundingBox();
			buffer->Material = mesh->getMeshBuffer(b)->getMaterial();
			clone->addMeshBuffer(buffer);
			buffer->drop();

			video::S3DVertex* v =
					(video::S3DVertex*)mesh->getMeshBuffer(b)->getVertices();

			u32 vertexCount = mesh->getMeshBuffer(b)->getVertexCount();

			indices = mesh->getMeshBuffer(b)->getIndices();
			indexCount = mesh->getMeshBuffer(b)->getIndexCount();
			outIdx = &buffer->Indices;

			buffer->Vertices.reallocate(vertexCount);

			for (u32 i=0; i < vertexCount; ++i)
			{
				bool found = false;
				for (u32 j=0; j < i; ++j)
				{
					if ( v[i].Pos.equals( v[j].Pos, tolerance) &&
						 v[i].Normal.equals( v[j].Normal, tolerance) &&
						 v[i].TCoords.equals( v[j].TCoords ) &&
						(v[i].Color == v[j].Color) )
					{
						redirects[i] = redirects[j];
						found = true;
						break;
					}
				}
				if (!found)
				{
					redirects[i] = buffer->Vertices.size();
					buffer->Vertices.push_back(v[i]);
				}
			}
			
			break;
		}
		case video::EVT_2TCOORDS:
		{
			SMeshBufferLightMap* buffer = new SMeshBufferLightMap();
			buffer->BoundingBox = mesh->getMeshBuffer(b)->getBoundingBox();
			buffer->Material = mesh->getMeshBuffer(b)->getMaterial();
			clone->addMeshBuffer(buffer);
			buffer->drop();

			video::S3DVertex2TCoords* v =
					(video::S3DVertex2TCoords*)mesh->getMeshBuffer(b)->getVertices();

			u32 vertexCount = mesh->getMeshBuffer(b)->getVertexCount();

			indices = mesh->getMeshBuffer(b)->getIndices();
			indexCount = mesh->getMeshBuffer(b)->getIndexCount();
			outIdx = &buffer->Indices;

			buffer->Vertices.reallocate(vertexCount);

			for (u32 i=0; i < vertexCount; ++i)
			{
				bool found = false;
				for (u32 j=0; j < i; ++j)
				{
					if ( v[i].Pos.equals( v[j].Pos, tolerance) &&
						 v[i].Normal.equals( v[j].Normal, tolerance) &&
						 v[i].TCoords.equals( v[j].TCoords ) &&
						 v[i].TCoords2.equals( v[j].TCoords2 ) &&
						(v[i].Color == v[j].Color) )
					{
						redirects[i] = redirects[j];
						found = true;
						break;
					}
				}
				if (!found)
				{
					redirects[i] = buffer->Vertices.size();
					buffer->Vertices.push_back(v[i]);
				}
			}
			break;
		}
		case video::EVT_TANGENTS:
		{
			SMeshBufferTangents* buffer = new SMeshBufferTangents();
			buffer->BoundingBox = mesh->getMeshBuffer(b)->getBoundingBox();
			buffer->Material = mesh->getMeshBuffer(b)->getMaterial();
			clone->addMeshBuffer(buffer);
			buffer->drop();

			video::S3DVertexTangents* v =
					(video::S3DVertexTangents*)mesh->getMeshBuffer(b)->getVertices();

			u32 vertexCount = mesh->getMeshBuffer(b)->getVertexCount();

			indices = mesh->getMeshBuffer(b)->getIndices();
			indexCount = mesh->getMeshBuffer(b)->getIndexCount();
			outIdx = &buffer->Indices;

			buffer->Vertices.reallocate(vertexCount);

			for (u32 i=0; i < vertexCount; ++i)
			{
				bool found = false;
				for (u32 j=0; j < i; ++j)
				{
					if ( v[i].Pos.equals( v[j].Pos, tolerance) &&
						 v[i].Normal.equals( v[j].Normal, tolerance) &&
						 v[i].TCoords.equals( v[j].TCoords ) &&
						 v[i].Tangent.equals( v[j].Tangent, tolerance ) &&
						 v[i].Binormal.equals( v[j].Binormal, tolerance ) &&
						(v[i].Color == v[j].Color) )
					{
						redirects[i] = redirects[j];
						found = true;
						break;
					}
				}
				if (!found)
				{
					redirects[i] = buffer->Vertices.size();
					buffer->Vertices.push_back(v[i]);
				}
			}
			break;
		}
		default:
			os::Printer::log("Cannot create welded mesh, vertex type unsupported", ELL_ERROR);
			break;
		}

		// write the buffer's index list
		core::array<u16> &Indices = *outIdx;

		Indices.set_used(indexCount);
		for (u32 i=0; i<indexCount; ++i)
		{
			Indices[i] = redirects[ indices[i] ];
		}
	}
	return clone;
}


//! Creates a copy of the mesh, which will only consist of S3DVertexTangents vertices.
IMesh* CMeshManipulator::createMeshWithTangents(IMesh* mesh, bool recalculateNormals, bool smooth, bool angleWeighted, bool calculateTangents) const
{
	if (!mesh)
		return 0;

	// copy mesh and fill data into SMeshBufferTangents

	SMesh* clone = new SMesh();
	const u32 meshBufferCount = mesh->getMeshBufferCount();

	for (u32 b=0; b<meshBufferCount; ++b)
	{
		const IMeshBuffer* original = mesh->getMeshBuffer(b);
		const u32 idxCnt = original->getIndexCount();
		const u16* idx = original->getIndices();

		SMeshBufferTangents* buffer = new SMeshBufferTangents();

		buffer->Material = original->getMaterial();
		buffer->Vertices.reallocate(idxCnt);
		buffer->Indices.set_used(idxCnt);

		core::map<video::S3DVertexTangents, int> vertMap;
		int vertLocation;

		// copy vertices

		const video::E_VERTEX_TYPE vType = original->getVertexType();
		video::S3DVertexTangents vNew;
		for (u32 i=0; i<idxCnt; ++i)
		{
			switch(vType)
			{
			case video::EVT_STANDARD:
				{
					const video::S3DVertex* v =
						(const video::S3DVertex*)original->getVertices();
					vNew = video::S3DVertexTangents(
							v[idx[i]].Pos, v[idx[i]].Normal, v[idx[i]].Color, v[idx[i]].TCoords);
				}
				break;
			case video::EVT_2TCOORDS:
				{
					const video::S3DVertex2TCoords* v =
						(const video::S3DVertex2TCoords*)original->getVertices();
					vNew = video::S3DVertexTangents(
							v[idx[i]].Pos, v[idx[i]].Normal, v[idx[i]].Color, v[idx[i]].TCoords);
				}
				break;
			case video::EVT_TANGENTS:
				{
					const video::S3DVertexTangents* v =
						(const video::S3DVertexTangents*)original->getVertices();
					vNew = v[idx[i]];
				}
				break;
			}
			core::map<video::S3DVertexTangents, int>::Node* n = vertMap.find(vNew);
			if (n)
			{
				vertLocation = n->getValue();
			}
			else
			{
				vertLocation = buffer->Vertices.size();
				buffer->Vertices.push_back(vNew);
				vertMap.insert(vNew, vertLocation);
			}

			// create new indices
			buffer->Indices[i] = vertLocation;
		}
		buffer->recalculateBoundingBox();

		// add new buffer
		clone->addMeshBuffer(buffer);
		buffer->drop();
	}

	clone->recalculateBoundingBox();
	if (calculateTangents)
		recalculateTangents(clone, recalculateNormals, smooth, angleWeighted);

	return clone;
}


//! Creates a copy of the mesh, which will only consist of S3DVertex2TCoords vertices.
IMesh* CMeshManipulator::createMeshWith2TCoords(IMesh* mesh) const
{
	if (!mesh)
		return 0;

	// copy mesh and fill data into SMeshBufferLightMap

	SMesh* clone = new SMesh();
	const u32 meshBufferCount = mesh->getMeshBufferCount();

	for (u32 b=0; b<meshBufferCount; ++b)
	{
		const IMeshBuffer* original = mesh->getMeshBuffer(b);
		const u32 idxCnt = original->getIndexCount();
		const u16* idx = original->getIndices();

		SMeshBufferLightMap* buffer = new SMeshBufferLightMap();
		buffer->Material = original->getMaterial();
		buffer->Vertices.reallocate(idxCnt);
		buffer->Indices.set_used(idxCnt);

		core::map<video::S3DVertex2TCoords, int> vertMap;
		int vertLocation;

		// copy vertices

		const video::E_VERTEX_TYPE vType = original->getVertexType();
		video::S3DVertex2TCoords vNew;
		for (u32 i=0; i<idxCnt; ++i)
		{
			switch(vType)
			{
			case video::EVT_STANDARD:
				{
					const video::S3DVertex* v =
						(const video::S3DVertex*)original->getVertices();
					vNew = video::S3DVertex2TCoords(
							v[idx[i]].Pos, v[idx[i]].Normal, v[idx[i]].Color, v[idx[i]].TCoords, v[idx[i]].TCoords);
				}
				break;
			case video::EVT_2TCOORDS:
				{
					const video::S3DVertex2TCoords* v =
						(const video::S3DVertex2TCoords*)original->getVertices();
					vNew = v[idx[i]];
				}
				break;
			case video::EVT_TANGENTS:
				{
					const video::S3DVertexTangents* v =
						(const video::S3DVertexTangents*)original->getVertices();
					vNew = video::S3DVertex2TCoords(
							v[idx[i]].Pos, v[idx[i]].Normal, v[idx[i]].Color, v[idx[i]].TCoords, v[idx[i]].TCoords);
				}
				break;
			}
			core::map<video::S3DVertex2TCoords, int>::Node* n = vertMap.find(vNew);
			if (n)
			{
				vertLocation = n->getValue();
			}
			else
			{
				vertLocation = buffer->Vertices.size();
				buffer->Vertices.push_back(vNew);
				vertMap.insert(vNew, vertLocation);
			}

			// create new indices
			buffer->Indices[i] = vertLocation;
		}
		buffer->recalculateBoundingBox();

		// add new buffer
		clone->addMeshBuffer(buffer);
		buffer->drop();
	}

	clone->recalculateBoundingBox();
	return clone;
}

//! Creates a copy of the mesh, which will only consist of S3DVertex vertices.
IMesh* CMeshManipulator::createMeshWith1TCoords(IMesh* mesh) const
{
	if (!mesh)
		return 0;

	// copy mesh and fill data into SMeshBuffer
	SMesh* clone = new SMesh();
	const u32 meshBufferCount = mesh->getMeshBufferCount();

	for (u32 b=0; b<meshBufferCount; ++b)
	{
		const IMeshBuffer* original = mesh->getMeshBuffer(b);
		const u32 idxCnt = original->getIndexCount();
		const u16* idx = original->getIndices();

		SMeshBuffer* buffer = new SMeshBuffer();
		buffer->Material = original->getMaterial();
		buffer->Vertices.reallocate(idxCnt);
		buffer->Indices.set_used(idxCnt);

		core::map<video::S3DVertex, int> vertMap;
		int vertLocation;

		// copy vertices
		const video::E_VERTEX_TYPE vType = original->getVertexType();
		video::S3DVertex vNew;
		for (u32 i=0; i<idxCnt; ++i)
		{
			switch(vType)
			{
			case video::EVT_STANDARD:
				{
					video::S3DVertex* v =
						(video::S3DVertex*)original->getVertices();
					vNew = v[idx[i]];
				}
				break;
			case video::EVT_2TCOORDS:
				{
					video::S3DVertex2TCoords* v =
						(video::S3DVertex2TCoords*)original->getVertices();
					vNew = video::S3DVertex(
							v[idx[i]].Pos, v[idx[i]].Normal, v[idx[i]].Color, v[idx[i]].TCoords);
				}
				break;
			case video::EVT_TANGENTS:
				{
					video::S3DVertexTangents* v =
						(video::S3DVertexTangents*)original->getVertices();
					vNew = video::S3DVertex(
							v[idx[i]].Pos, v[idx[i]].Normal, v[idx[i]].Color, v[idx[i]].TCoords);
				}
				break;
			}
			core::map<video::S3DVertex, int>::Node* n = vertMap.find(vNew);
			if (n)
			{
				vertLocation = n->getValue();
			}
			else
			{
				vertLocation = buffer->Vertices.size();
				buffer->Vertices.push_back(vNew);
				vertMap.insert(vNew, vertLocation);
			}

			// create new indices
			buffer->Indices[i] = vertLocation;
		}
		buffer->recalculateBoundingBox();

		// add new buffer
		clone->addMeshBuffer(buffer);
		buffer->drop();
	}

	clone->recalculateBoundingBox();
	return clone;
}


void CMeshManipulator::calculateTangents(
	core::vector3df& normal,
	core::vector3df& tangent,
	core::vector3df& binormal,
	const core::vector3df& vt1, const core::vector3df& vt2, const core::vector3df& vt3, // vertices
	const core::vector2df& tc1, const core::vector2df& tc2, const core::vector2df& tc3) // texture coords
{
	// choose one of them:
	//#define USE_NVIDIA_GLH_VERSION // use version used by nvidia in glh headers
	#define USE_IRR_VERSION

#ifdef USE_IRR_VERSION

	core::vector3df v1 = vt1 - vt2;
	core::vector3df v2 = vt3 - vt1;
	normal = v2.crossProduct(v1);
	normal.normalize();

	// binormal

	f32 deltaX1 = tc1.X - tc2.X;
	f32 deltaX2 = tc3.X - tc1.X;
	binormal = (v1 * deltaX2) - (v2 * deltaX1);
	binormal.normalize();

	// tangent

	f32 deltaY1 = tc1.Y - tc2.Y;
	f32 deltaY2 = tc3.Y - tc1.Y;
	tangent = (v1 * deltaY2) - (v2 * deltaY1);
	tangent.normalize();

	// adjust

	core::vector3df txb = tangent.crossProduct(binormal);
	if (txb.dotProduct(normal) < 0.0f)
	{
		tangent *= -1.0f;
		binormal *= -1.0f;
	}

#endif // USE_IRR_VERSION

#ifdef USE_NVIDIA_GLH_VERSION

	tangent.set(0,0,0);
	binormal.set(0,0,0);

	core::vector3df v1(vt2.X - vt1.X, tc2.X - tc1.X, tc2.Y - tc1.Y);
	core::vector3df v2(vt3.X - vt1.X, tc3.X - tc1.X, tc3.Y - tc1.Y);

	core::vector3df txb = v1.crossProduct(v2);
	if ( !core::iszero ( txb.X ) )
	{
		tangent.X  = -txb.Y / txb.X;
		binormal.X = -txb.Z / txb.X;
	}

	v1.X = vt2.Y - vt1.Y;
	v2.X = vt3.Y - vt1.Y;
	txb = v1.crossProduct(v2);

	if ( !core::iszero ( txb.X ) )
	{
		tangent.Y  = -txb.Y / txb.X;
		binormal.Y = -txb.Z / txb.X;
	}

	v1.X = vt2.Z - vt1.Z;
	v2.X = vt3.Z - vt1.Z;
	txb = v1.crossProduct(v2);

	if ( !core::iszero ( txb.X ) )
	{
		tangent.Z  = -txb.Y / txb.X;
		binormal.Z = -txb.Z / txb.X;
	}

	tangent.normalize();
	binormal.normalize();

	normal = tangent.crossProduct(binormal);
	normal.normalize();

	binormal = tangent.crossProduct(normal);
	binormal.normalize();

	core::plane3d<f32> pl(vt1, vt2, vt3);

	if(normal.dotProduct(pl.Normal) < 0.0f )
		normal *= -1.0f;

#endif // USE_NVIDIA_GLH_VERSION
}



//! Returns amount of polygons in mesh.
s32 CMeshManipulator::getPolyCount(scene::IMesh* mesh) const
{
	if (!mesh)
		return 0;

	s32 trianglecount = 0;

	for (u32 g=0; g<mesh->getMeshBufferCount(); ++g)
		trianglecount += mesh->getMeshBuffer(g)->getIndexCount() / 3;

	return trianglecount;
}


//! Returns amount of polygons in mesh.
s32 CMeshManipulator::getPolyCount(scene::IAnimatedMesh* mesh) const
{
	if (mesh && mesh->getFrameCount() != 0)
		return getPolyCount(mesh->getMesh(0));

	return 0;
}


//! create a new AnimatedMesh and adds the mesh to it
IAnimatedMesh * CMeshManipulator::createAnimatedMesh(scene::IMesh* mesh, scene::E_ANIMATED_MESH_TYPE type) const
{
	return new SAnimatedMesh(mesh, type);
}


} // end namespace scene
} // end namespace irr

