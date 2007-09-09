#include "CIrrMeshWriter.h"
#include "os.h"
#include "IXMLWriter.h"
#include "IMesh.h"
#include "IAttributes.h"

namespace irr
{
namespace scene
{


CIrrMeshWriter::CIrrMeshWriter(irr::video::IVideoDriver* driver,
				irr::io::IFileSystem* fs)
	: FileSystem(fs), VideoDriver(driver), Writer(0)
{
	if (VideoDriver)
		VideoDriver->grab();

	if (FileSystem)
		FileSystem->grab();
}


CIrrMeshWriter::~CIrrMeshWriter()
{
	if (VideoDriver)
		VideoDriver->drop();

	if (FileSystem)
		FileSystem->drop();
}


//! Returns the type of the mesh writer
EMESH_WRITER_TYPE CIrrMeshWriter::getType() const
{
	return EMWT_IRR_MESH;
}


//! writes a mesh
bool CIrrMeshWriter::writeMesh(io::IWriteFile* file, scene::IMesh* mesh, s32 flags)
{
	if (!file)
		return false;

	Writer = FileSystem->createXMLWriter(file);

	if (!Writer)
	{
		os::Printer::log("Could not write file", file->getFileName());
		return false;
	}

	os::Printer::log("Writing mesh", file->getFileName());

	// write IRR MESH header

	Writer->writeXMLHeader();

	Writer->writeElement(L"mesh", false,
		L"xmlns", L"http://irrlicht.sourceforge.net/IRRMESH_09_2007",
		L"version", L"1.0");
	Writer->writeLineBreak();

	// add some informational comment. Add a space after and before the comment
	// tags so that some braindead xml parsers (AS anyone?) are able to parse this too.

	core::stringw infoComment = L" This file contains a static mesh in the Irrlicht Engine format with ";
	infoComment += core::stringw(mesh->getMeshBufferCount());
	infoComment += L" materials.";

	Writer->writeComment(infoComment.c_str());
	Writer->writeLineBreak();

	// write mesh bounding box

	writeBoundingBox(mesh->getBoundingBox());
	Writer->writeLineBreak();

	// write mesh buffers
	
	for (int i=0; i<(int)mesh->getMeshBufferCount(); ++i)
	{
		irr::scene::IMeshBuffer* buffer = mesh->getMeshBuffer(i);
		if (buffer)
		{
			writeMeshBuffer(buffer);
			Writer->writeLineBreak();
		}
	}

	Writer->writeClosingTag(L"mesh");

	Writer->drop();
	return true;
}


void CIrrMeshWriter::writeBoundingBox(const core::aabbox3df& box)
{
	Writer->writeElement(L"boundingBox", true,
        L"minEdge", getVectorAsStringLine(box.MinEdge).c_str(),
		L"maxEdge", getVectorAsStringLine(box.MaxEdge).c_str()	);
}


core::stringw CIrrMeshWriter::getVectorAsStringLine(const core::vector3df& v)
{
	core::stringw str;

	str = core::stringw(v.X);
	str += L" ";
	str += core::stringw(v.Y);
	str += L" ";
	str += core::stringw(v.Z);
	
	return str;
}


core::stringw CIrrMeshWriter::getVectorAsStringLine(const core::vector2df& v)
{
	core::stringw str;

	str = core::stringw(v.X);
	str += L" ";
	str += core::stringw(v.Y);
	
	return str;
}


void CIrrMeshWriter::writeMeshBuffer(const scene::IMeshBuffer* buffer)
{
	Writer->writeElement(L"buffer", false);
	Writer->writeLineBreak();

	// write bounding box

	writeBoundingBox(buffer->getBoundingBox());
	Writer->writeLineBreak();

	// write material

	writeMaterial(buffer->getMaterial());

	// write vertices

	const wchar_t* vertexTypeStr;

	switch(buffer->getVertexType())
	{
	case video::EVT_STANDARD:
		vertexTypeStr = L"standard";
		break;
	case video::EVT_TANGENTS:
		vertexTypeStr = L"tangents";
		break;
	case video::EVT_2TCOORDS:
	default:
		vertexTypeStr = L"2tcoords";
		break;
	}

	Writer->writeElement(L"vertices", false, 
		L"type", vertexTypeStr,
		L"vertexCount", core::stringw(buffer->getVertexCount()).c_str());

	Writer->writeLineBreak();

	int vertexCount = (int)buffer->getVertexCount();

	switch(buffer->getVertexType())
	{
	case video::EVT_STANDARD:
		{
			video::S3DVertex* vtx = (video::S3DVertex*)buffer->getVertices();
			for (int j=0; j<vertexCount; ++j)
			{
				core::stringw str = getVectorAsStringLine(vtx[j].Pos);
				str += L" ";
				str += getVectorAsStringLine(vtx[j].Normal);

				char tmp[12];
				sprintf(tmp, " %08x ", vtx[j].Color.color);
				str += tmp;

				str += getVectorAsStringLine(vtx[j].TCoords);

                Writer->writeText(str.c_str());
				Writer->writeLineBreak();
			}
		}
		break;
	case video::EVT_2TCOORDS:
		{
			video::S3DVertex2TCoords* vtx = (video::S3DVertex2TCoords*)buffer->getVertices();
			for (int j=0; j<vertexCount; ++j)
			{
				core::stringw str = getVectorAsStringLine(vtx[j].Pos);
				str += L" ";
				str += getVectorAsStringLine(vtx[j].Normal);

				char tmp[12];
				sprintf(tmp, " %08x ", vtx[j].Color.color);
				str += tmp;

				str += getVectorAsStringLine(vtx[j].TCoords);
				str += L" ";
				str += getVectorAsStringLine(vtx[j].TCoords2);

                Writer->writeText(str.c_str());
				Writer->writeLineBreak();
			}
		}
		break;
	case video::EVT_TANGENTS:
		{
			video::S3DVertexTangents* vtx = (video::S3DVertexTangents*)buffer->getVertices();
			for (int j=0; j<vertexCount; ++j)
			{
				core::stringw str = getVectorAsStringLine(vtx[j].Pos);
				str += L" ";
				str += getVectorAsStringLine(vtx[j].Normal);

				char tmp[12];
				sprintf(tmp, " %08x ", vtx[j].Color.color);
				str += tmp;

				str += getVectorAsStringLine(vtx[j].TCoords);
				str += L" ";
				str += getVectorAsStringLine(vtx[j].Tangent);
				str += L" ";
				str += getVectorAsStringLine(vtx[j].Binormal);

                Writer->writeText(str.c_str());
				Writer->writeLineBreak();
			}
		}
		break;
	}

	Writer->writeClosingTag(L"vertices");
	Writer->writeLineBreak();

	// write indices

	Writer->writeElement(L"indices", false, 
		L"indexCount", core::stringw(buffer->getIndexCount()).c_str());

	Writer->writeLineBreak();

	int indexCount = (int)buffer->getIndexCount();
	const u16* idx = buffer->getIndices();
	const int maxIndicesPerLine = 25;

	for (int i=0; i<indexCount; ++i)
	{
		core::stringw str((int)idx[i]);
		Writer->writeText(str.c_str());
		
		if (i % maxIndicesPerLine != maxIndicesPerLine)
		{
			if (i % maxIndicesPerLine == maxIndicesPerLine-1)
				Writer->writeLineBreak();
			else
				Writer->writeText(L" ");
		}
	}

	if ((indexCount-1) % maxIndicesPerLine != maxIndicesPerLine-1)
		Writer->writeLineBreak();

	Writer->writeClosingTag(L"indices");
	Writer->writeLineBreak();

	// close buffer tag

	Writer->writeClosingTag(L"buffer");
}


void CIrrMeshWriter::writeMaterial(const video::SMaterial& material)
{
	// simply use irrlichts built-in attribute serialization capabilities here:

	irr::io::IAttributes* attributes = 
		VideoDriver->createAttributesFromMaterial(material);

	if (attributes)
	{
		attributes->write(Writer, false, L"material");
		attributes->drop();
	}
}


} // end namespace
} // end namespace