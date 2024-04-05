/***********************************************************************************************************************
*
* Copyright (c) 2017 by Tech Soft 3D, LLC.
* The information contained herein is confidential and proprietary to Tech Soft 3D, LLC., and considered a trade secret
* as defined under civil and criminal statutes. Tech Soft 3D shall pursue its civil and criminal remedies in the event
* of unauthorized use or misappropriation of its trade secrets. Use of this information by anyone other than authorized
* employees of Tech Soft 3D, LLC. is granted only under a written non-disclosure agreement, expressly prescribing the
* scope and manner of such use.
*
***********************************************************************************************************************/

/*
* This sample demonstrates how to load a model and export it as a model file of a different format. The chosen
* format is determined by the file extension of the output file name.
*/

#define INITIALIZE_A3D_API
#include <A3DSDKIncludes.h>
#include "libconverter.h"

#include "common.hpp"
#include <sstream>
#include <fstream>
#include <map>
#include <sys/stat.h>
#include <string>
#include "utilities.h"
#include "Matrix.h"

static MY_CHAR acSrcFileName[_MAX_PATH * 2];
static MY_CHAR acXmlFileName[_MAX_PATH * 2];
static MY_CHAR acScDirPath[_MAX_PATH * 2] = { '\0' };
static MY_CHAR acScsDirPath[_MAX_PATH * 2] = { '\0' };
static MY_CHAR acPrcDirPath[_MAX_PATH * 2] = { '\0' };
static A3DUTF8Char acModelNameUTF8[_MAX_PATH * 2];
static MY_CHAR* pcScModelDirPath = NULL;
static MY_CHAR* pcScsModelDirPath = NULL;
static MY_CHAR *pcPrcModelDirPath = NULL;
static Communicator::Importer libImporter;

A3DAsmModelFileData _sModelFileData;
std::ofstream _ofs;
std::map<A3DAsmProductOccurrence*, int> _componentIdMap;
std::map<std::string, int> _partNameMap;
int _iComponentId = 0;

void addSeparator(MY_CHAR* pcDirPath)
{
	if (MY_STRLEN(pcDirPath))
	{
		size_t lastId = MY_STRLEN(pcDirPath) - 1;
#ifdef _MSC_VER
		if ('\\' != pcDirPath[lastId])
			MY_STRCAT(pcDirPath, _T("\\"));
#else
		if ('/' != pcDirPath[lastId])
			MY_STRCAT(pcDirPath, "/");
#endif
	}
}

bool savePrcSc(const MY_CHAR* partName, A3DAsmModelFile* pModelFile)
{
	// Export PRC
	if (pcPrcModelDirPath)
	{
		A3DRWParamsExportPrcData sExportData;
		A3DRWParamsPrcWriteHelper* pWriteHelprt;
		A3D_INITIALIZE_DATA(A3DRWParamsExportPrcData, sExportData);

		MY_CHAR* pcPrcFilePath = new MY_CHAR[(MY_STRLEN(pcPrcModelDirPath) + MY_STRLEN(partName) + 6) * sizeof(MY_CHAR)]();

#ifdef _MSC_VER
		wsprintf(pcPrcFilePath, _T("%s\\%s.prc"), pcPrcModelDirPath, partName);

		A3DUTF8Char* pcPrcFilePathUTF8 = new A3DUTF8Char[(MY_STRLEN(pcPrcFilePath) + 1) * sizeof(A3DUniChar)]();
		A3DMiscUTF16ToUTF8(pcPrcFilePath, pcPrcFilePathUTF8);

		if (A3D_SUCCESS != A3DAsmModelFileExportToPrcFile(pModelFile, &sExportData, pcPrcFilePathUTF8, &pWriteHelprt))
			return false;
#else
		sprintf(pcPrcFilePath, "%s/%s.prc", pcPrcModelDirPath, partName);

		if (A3D_SUCCESS != A3DAsmModelFileExportToPrcFile(pModelFile, &sExportData, pcPrcFilePath, &pWriteHelprt))
			return false;
#endif
	}

	// Export SCZ and SCS
	if (!libImporter.Load(pModelFile))
		return false;

	Communicator::Exporter exporter; // Export Initialization
	if (!exporter.Init(&libImporter))
		return false;

	Communicator::SC_Export_Options exportOptions; // Export Stream Cache Model
	exportOptions.sc_create_scz = true;
	exportOptions.export_attributes = true;

	A3DUTF8Char* pcScFilePathUTF8 = nullptr;
	A3DUTF8Char* pcScsFilePathUTF8 = nullptr;

#ifdef _MSC_VER
	if (pcScModelDirPath)
	{
		MY_CHAR* pcScFilePath = new MY_CHAR[(MY_STRLEN(pcScModelDirPath) + MY_STRLEN(partName) + 2) * sizeof(MY_CHAR)]();
		wsprintf(pcScFilePath, _T("%s\\%s"), pcScModelDirPath, partName);

		pcScFilePathUTF8 = (A3DUTF8Char*)malloc(MY_STRLEN(pcScFilePath) * sizeof(A3DUniChar));
		A3DMiscUTF16ToUTF8(pcScFilePath, pcScFilePathUTF8);
	}

	if (pcScsModelDirPath)
	{
		MY_CHAR* pcScsFilePath = new MY_CHAR[(MY_STRLEN(pcScsModelDirPath) + MY_STRLEN(partName) + 6) * sizeof(MY_CHAR)]();
		wsprintf(pcScsFilePath, _T("%s\\%s.scs"), pcScsModelDirPath, partName);

		pcScsFilePathUTF8 = (A3DUTF8Char*)malloc(MY_STRLEN(pcScsFilePath) * sizeof(A3DUniChar));
		A3DMiscUTF16ToUTF8(pcScsFilePath, pcScsFilePathUTF8);
	}
#else
	if (pcScModelDirPath)
	{
		pcScFilePathUTF8 = (A3DUTF8Char*)malloc((MY_STRLEN(pcScModelDirPath) + MY_STRLEN(partName) + 2) * sizeof(A3DUTF8Char));
		sprintf(pcScFilePathUTF8, "%s/%s", pcScModelDirPath, partName);
	}

	if (pcScsModelDirPath)
	{
		pcScsFilePathUTF8 = (A3DUTF8Char*)malloc((MY_STRLEN(pcScsModelDirPath) + MY_STRLEN(partName) + 6) * sizeof(A3DUTF8Char));
		sprintf(pcScsFilePathUTF8, "%s/%s.scs", pcScsModelDirPath, partName);
	}
#endif
	bool bRet;
	if (pcScFilePathUTF8 || pcScsFilePathUTF8)
	{
		if (!exporter.WriteSC(pcScFilePathUTF8, pcScsFilePathUTF8, exportOptions))
			return false;
	}

	return true;
}

void traverseTree(A3DTree* const hnd_tree, A3DTreeNode* const hnd_node) {
	A3DStatus iRet;

	// Get node name
	A3DUTF8Char* node_name = nullptr;
	if (A3D_SUCCESS != A3DTreeNodeGetName(hnd_node, &node_name))
		node_name = nullptr;

	A3DEntity* pEntity = nullptr;
	A3DTreeNodeGetEntity(hnd_node, &pEntity);
	A3DEEntityType eType = kA3DTypeUnknown;
	if (A3D_SUCCESS == A3DEntityGetType(pEntity, &eType))
	{
		const A3DUTF8Char* pTypeMsg = A3DMiscGetEntityTypeMsg(eType);

		if (0 == strcmp(pTypeMsg, "kA3DTypeAsmProductOccurrence"))
		{
			// Get the ProductOccurrence
			A3DAsmProductOccurrence* pPO = (A3DAsmProductOccurrence*)pEntity;

			A3DAsmProductOccurrenceData sData;
			A3D_INITIALIZE_DATA(A3DAsmProductOccurrenceData, sData);
			if (A3D_SUCCESS == A3DAsmProductOccurrenceGet(pPO, &sData))
			{
				int thisId;
				if (_componentIdMap.count(pPO))
					thisId = _componentIdMap[pPO];
				else
					thisId = _iComponentId++;

				// XML
				_ofs << "\t\t\t<ProductOccurence Id=\"" << thisId << "\" Name=\"";

				if (nullptr != node_name)
				{
					char escapeText[_MAX_PATH * 2] = { '\0' };
					xmlEscape(node_name, escapeText);

					_ofs << escapeText;
				}

				_ofs << "\" ";
				_ofs << "Unit=\"" << sData.m_dUnit << "\" ";

				// Output children list
				if (0 < sData.m_uiPOccurrencesSize)
				{
					_ofs << "Children=\"";

					for (int i = 0; i < sData.m_uiPOccurrencesSize; i++)
					{
						if (0 < i)
							_ofs << " ";

						_ofs << _iComponentId;
						_componentIdMap[sData.m_ppPOccurrences[i]] = _iComponentId++;
					}

					_ofs << "\" ";
				}

				// Check whether a new part's PO
				bool bIsNewPart = false;
				if (NULL != sData.m_pPrototype || NULL != sData.m_pPart)
				{
					char refId[256] = { "\0" };
					if (NULL != sData.m_pPart)
					{
						if (0 == _componentIdMap.count(sData.m_pPart))
						{
							bIsNewPart = true;
							_componentIdMap[sData.m_pPart] = _iComponentId;
						}
						sprintf(refId, "%d", _componentIdMap[sData.m_pPart]);
					}
					else if (NULL != sData.m_pPrototype)
					{
						A3DAsmProductOccurrenceData sProtoData;
						A3D_INITIALIZE_DATA(A3DAsmProductOccurrenceData, sProtoData);
						iRet = A3DAsmProductOccurrenceGet(sData.m_pPrototype, &sProtoData);

						if (0 == sProtoData.m_uiPOccurrencesSize)
						{
							A3DEntity* pProto = sData.m_pPrototype;

							// Find parent prototype of the part
							while (sProtoData.m_pPrototype)
							{
								pProto = sProtoData.m_pPrototype;
								iRet = A3DAsmProductOccurrenceGet(sProtoData.m_pPrototype, &sProtoData);
							}

							// Check whether the prototype has a part
							if (sProtoData.m_pPart)
							{
								if (0 == _componentIdMap.count(pProto))
								{
									bIsNewPart = true;
									_componentIdMap[pProto] = _iComponentId;
								}
								sprintf(refId, "%d", _componentIdMap[pProto]);
							}
						}
					}

					if (strlen(refId))
					{
						_ofs << "InstanceRef=\"" << refId << "\" ";
					}
				}

				_ofs << "LayerId=\"65535\" Style=\"65535\" Behaviour=\"1\" ExchangeId=\"\" isPart=\"false\"";

				// Transformation
				A3DMiscTransformation* pTransf;
				if (A3D_SUCCESS == A3DTreeNodeGetTransformation(hnd_node, &pTransf))
				{
					if (NULL == pTransf && NULL != sData.m_pPrototype)
					{
						A3DAsmProductOccurrenceData sProtoData;
						A3D_INITIALIZE_DATA(A3DAsmProductOccurrenceData, sProtoData);
						A3DAsmProductOccurrenceGet(sData.m_pPrototype, &sProtoData);
						pTransf = sProtoData.m_pLocation;
					}
				}
				if (pTransf)
				{
					A3DMatrix4x4 sMatrix = A3DMatrix4x4();

					A3DEEntityType eType = kA3DTypeUnknown;
					iRet = A3DEntityGetType(pTransf, &eType);
					if (eType == kA3DTypeMiscCartesianTransformation)
					{
						A3DMiscCartesianTransformationData sTransData;
						A3D_INITIALIZE_DATA(A3DMiscCartesianTransformationData, sTransData);
						iRet = A3DMiscCartesianTransformationGet(pTransf, &sTransData);

						double dMirror = (sTransData.m_ucBehaviour & kA3DTransformationMirror) ? -1. : 1.;

						A3DVector3dData sZVector;
						sZVector = CrossProduct(&(sTransData.m_sXVector), &(sTransData.m_sYVector));

						sMatrix.m_adM[12] = sTransData.m_sOrigin.m_dX;
						sMatrix.m_adM[13] = sTransData.m_sOrigin.m_dY;
						sMatrix.m_adM[14] = sTransData.m_sOrigin.m_dZ;

						sMatrix.m_adM[0] = sTransData.m_sXVector.m_dX * sTransData.m_sScale.m_dX;
						sMatrix.m_adM[1] = sTransData.m_sXVector.m_dY * sTransData.m_sScale.m_dX;
						sMatrix.m_adM[2] = sTransData.m_sXVector.m_dZ * sTransData.m_sScale.m_dX;

						sMatrix.m_adM[4] = sTransData.m_sYVector.m_dX * sTransData.m_sScale.m_dY;
						sMatrix.m_adM[5] = sTransData.m_sYVector.m_dY * sTransData.m_sScale.m_dY;
						sMatrix.m_adM[6] = sTransData.m_sYVector.m_dZ * sTransData.m_sScale.m_dY;

						sMatrix.m_adM[8] = dMirror * sZVector.m_dX * sTransData.m_sScale.m_dZ;
						sMatrix.m_adM[9] = dMirror * sZVector.m_dY * sTransData.m_sScale.m_dZ;
						sMatrix.m_adM[10] = dMirror * sZVector.m_dZ * sTransData.m_sScale.m_dZ;

						sMatrix.m_adM[15] = 1.;

						iRet = A3DMiscCartesianTransformationGet(NULL, &sTransData);
					}
					else if (eType == kA3DTypeMiscGeneralTransformation)
					{
						A3DMiscGeneralTransformationData sTransData;
						A3D_INITIALIZE_DATA(A3DMiscGeneralTransformationData, sTransData);
						iRet = A3DMiscGeneralTransformationGet(pTransf, &sTransData);

						for (int i = 0; i < 16; i++)
							sMatrix.m_adM[i] = sTransData.m_adCoeff[i];

						iRet = A3DMiscGeneralTransformationGet(NULL, &sTransData);
					}

					_ofs << " >" << std::endl;
					_ofs << "\t\t\t\t<Transformation RelativeTransfo=\"" <<
						sMatrix.m_adM[0] << " " << sMatrix.m_adM[1] << " " << sMatrix.m_adM[2] << " " << sMatrix.m_adM[3] << " " <<
						sMatrix.m_adM[4] << " " << sMatrix.m_adM[5] << " " << sMatrix.m_adM[6] << " " << sMatrix.m_adM[7] << " " <<
						sMatrix.m_adM[8] << " " << sMatrix.m_adM[9] << " " << sMatrix.m_adM[10] << " " << sMatrix.m_adM[11] << " " <<
						sMatrix.m_adM[12] << " " << sMatrix.m_adM[13] << " " << sMatrix.m_adM[14] << " " << sMatrix.m_adM[15] << "\" />" << std::endl;
					_ofs << "\t\t\t</ProductOccurence>" << std::endl;
				}
				else
				{
					_ofs << " />" << std::endl;
				}

				// Export part data
				if (bIsNewPart)
				{
					A3DUTF8Char acFileNameUTF8[_MAX_PATH * 2 - 12] = { '\0' };
					if (nullptr != node_name)
						fNameEscape(node_name, acFileNameUTF8);
					else
						sprintf(acFileNameUTF8, "%d", _iComponentId);

					// Make uniqu name
					A3DUTF8Char acUniquNameUTF8[_MAX_PATH * 2];
					if (0 == _partNameMap[acFileNameUTF8])
					{
						_partNameMap[acFileNameUTF8] = 1;

						strcpy(acUniquNameUTF8, acFileNameUTF8);
					}
					else
					{
						_partNameMap[acFileNameUTF8]++;

						sprintf(acUniquNameUTF8, "%s-%d", acFileNameUTF8, _partNameMap[acFileNameUTF8]);
					}

					// Create a copy of the parent PO
					A3DAsmProductOccurrence* pCopyPO;
					if (A3D_SUCCESS == A3DAsmProductOccurrenceDeepCopy(pPO, &pCopyPO))
					{

						// Delete the parent's location since it is output to XML
						A3DAsmProductOccurrenceData sCopyData;
						A3D_INITIALIZE_DATA(A3DAsmProductOccurrenceData, sCopyData);
						A3DAsmProductOccurrenceGet(pCopyPO, &sCopyData);
						if (sCopyData.m_pLocation)
						{
							sCopyData.m_pLocation = NULL;
							iRet = A3DAsmProductOccurrenceEdit(&sCopyData, pCopyPO);
						}

						// Create a ModelFile of the part
						A3DAsmModelFileData sCopyModelFileData;
						A3D_INITIALIZE_DATA(A3DAsmModelFileData, sCopyModelFileData);
						sCopyModelFileData.m_uiPOccurrencesSize = 1;
						sCopyModelFileData.m_ppPOccurrences = &pCopyPO;
						sCopyModelFileData.m_dUnit = _sModelFileData.m_dUnit;
						sCopyModelFileData.m_eModellerType = _sModelFileData.m_eModellerType;
						sCopyModelFileData.m_pBIMData = _sModelFileData.m_pBIMData;

						A3DAsmModelFile* pCopyModelFile;
						iRet = A3DAsmModelFileCreate(&sCopyModelFileData, &pCopyModelFile);

						// Get RootBaseData of the PO and set
						A3DRootBaseData sRootBaseData;
						A3D_INITIALIZE_DATA(A3DRootBaseData, sRootBaseData);
						A3DRootBaseGet(pPO, &sRootBaseData);

						iRet = A3DRootBaseSet(pCopyModelFile, &sRootBaseData);

						A3DRootBaseGet(NULL, &sRootBaseData);

						// Save PRC and SC
#ifdef _MSC_VER
						MY_CHAR* pcFileName = new MY_CHAR[(strlen(acUniquNameUTF8) + 1) * sizeof(MY_CHAR)]();

						A3DMiscUTF8ToUTF16(acUniquNameUTF8, pcFileName);

						savePrcSc(pcFileName, pCopyModelFile);
#else
						savePrcSc(acUniquNameUTF8, pCopyModelFile);
#endif

						// XMML
						_ofs << "\t\t\t";
						_ofs << "<ProductOccurence Id=\"" << _iComponentId << "\" Name=\"\" ExchangeId=\"\" Behaviour=\"1\" IsPart=\"false\">" << std::endl;

						_ofs << "\t\t\t\t";
						_ofs << "<ExternalModel Name = \"" << acModelNameUTF8 << "/" << acUniquNameUTF8 << "\" />" << std::endl;

						_ofs << "\t\t\t";
						_ofs << "</ProductOccurence>" << std::endl;

						_iComponentId++;
					}
				}

				//Recursively call traverseTree on child nodes
				if (0 < sData.m_uiPOccurrencesSize)
				{
					// Get child nodes
					A3DUns32 n_child_nodes = 0;
					A3DTreeNode** child_nodes = nullptr;
					iRet = A3DTreeNodeGetChildren(hnd_tree, hnd_node, &n_child_nodes, &child_nodes);

					for (A3DUns32 n = 0; n < n_child_nodes; ++n)
					{
						traverseTree(hnd_tree, child_nodes[n]);
					}
					A3DTreeNodeGetChildren(0, 0, &n_child_nodes, &child_nodes);
				}

				A3DAsmProductOccurrenceGet(NULL, &sData);
			}
		}
	}
}

//######################################################################################################################
#ifdef _MSC_VER
int wmain(A3DInt32 iArgc, A3DUniChar** ppcArgv)
#else
int main(A3DInt32 iArgc, A3DUTF8Char** ppcArgv)
#endif
{
	//
	// ### COMMAND LINE ARGUMENTS
	//

	if (iArgc < 3)
	{
		MY_PRINTF2("Usage:\n %s [input CAD file] [output XML file] [output SC path] [output SCS path] [output PRC path]\n", ppcArgv[0]);
		return A3D_ERROR;
	}

	MY_STRCPY(acSrcFileName, ppcArgv[1]);
	MY_STRCPY(acXmlFileName, ppcArgv[2]);
	if (3 < iArgc)
		MY_STRCPY(acScDirPath, ppcArgv[3]);
	if (4 < iArgc)
		MY_STRCPY(acScsDirPath, ppcArgv[4]);
	if (5 < iArgc)
		MY_STRCPY(acPrcDirPath, ppcArgv[5]);

	// Add separator at end of the dist paths
	addSeparator(acScDirPath);
	addSeparator(acScsDirPath);
	addSeparator(acPrcDirPath);

	//
	// ### INITIALIZE HOOPS EXCHANGE
	//
	A3DStatus iRet;
	
	if (!A3DSDKLoadLibrary(_T("")))
		return 0;

	iRet = A3DLicPutUnifiedLicense(HOOPS_LICENSE);
	if (iRet != A3D_SUCCESS)
		return 0;

	A3DInt32 iMajorVersion = 0, iMinorVersion = 0;
	iRet = A3DDllGetVersion(&iMajorVersion, &iMinorVersion);
	if (iRet != A3D_SUCCESS)
		return 0;

	iRet = A3DDllInitialize(A3D_DLL_MAJORVERSION, A3D_DLL_MINORVERSION);
	if (iRet != A3D_SUCCESS)
		return iRet;

	//
	// ### INITIALIZE HOOPS COMMUNICATOR LIBCONVERTER
	//
	Communicator::Converter libConverter;

	libConverter.Init(HOOPS_LICENSE);

	if (!libImporter.Init(&libConverter))
		return 0;

	//
	// ### MODEL IMPORT
	//
	A3DRWParamsLoadData m_sLoadData;
	A3DAsmModelFile* pModelFile;

	// Init import options
	A3D_INITIALIZE_DATA(A3DRWParamsLoadData, m_sLoadData);
	m_sLoadData.m_sGeneral.m_bReadSolids = true;
	m_sLoadData.m_sGeneral.m_bReadSurfaces = true;
	m_sLoadData.m_sGeneral.m_bReadWireframes = true;
	m_sLoadData.m_sGeneral.m_bReadPmis = true;
	m_sLoadData.m_sGeneral.m_bReadAttributes = true;
	m_sLoadData.m_sGeneral.m_bReadHiddenObjects = false;
	m_sLoadData.m_sGeneral.m_bReadConstructionAndReferences = false;
	m_sLoadData.m_sGeneral.m_bReadActiveFilter = false;
	m_sLoadData.m_sGeneral.m_eReadingMode2D3D = kA3DRead_3D;
	m_sLoadData.m_sGeneral.m_eReadGeomTessMode = kA3DReadGeomAndTess;
	m_sLoadData.m_sGeneral.m_eDefaultUnit = kA3DUnitUnknown;
	m_sLoadData.m_sTessellation.m_eTessellationLevelOfDetail = kA3DTessLODMedium;
	m_sLoadData.m_sAssembly.m_bUseRootDirectory = true;
	m_sLoadData.m_sMultiEntries.m_bLoadDefault = true;
	m_sLoadData.m_sPmi.m_bAlwaysSubstituteFont = false;
	m_sLoadData.m_sPmi.m_pcSubstitutionFont = (char*)"Myriad CAD";

	m_sLoadData.m_sSpecifics.m_sRevit.m_eMultiThreadedMode = kA3DRevitMultiThreadedMode_Disabled;

#ifdef _MSC_VER
	A3DUTF8Char* pcFileNameUTF8 = new A3DUTF8Char[(MY_STRLEN(acSrcFileName) + 1) * sizeof(A3DUniChar)]();
	A3DMiscUTF16ToUTF8(acSrcFileName, pcFileNameUTF8);

	iRet = A3DAsmModelFileLoadFromFile(pcFileNameUTF8, &m_sLoadData, &pModelFile);
	if (iRet != A3D_SUCCESS)
		return 0;

	getFileBodyName(pcFileNameUTF8, acModelNameUTF8);
#else
	iRet = A3DAsmModelFileLoadFromFile(acSrcFileName, &m_sLoadData, &pModelFile);
	if (iRet != A3D_SUCCESS)
		return 0;

	getFileBodyName(acSrcFileName, acModelNameUTF8);
#endif

	// Make dirs
#ifdef _MSC_VER
	A3DUniChar* pcModelName = new A3DUniChar[(strlen(acModelNameUTF8) + 1) * sizeof(A3DUTF8Char)]();
	A3DMiscUTF8ToUnicode(acModelNameUTF8, (A3DUTF8Char*)pcModelName);
	// SC
	if (MY_STRLEN(acScDirPath))
	{
		size_t len = MY_STRLEN(acScDirPath) + MY_STRLEN(pcModelName) + 1;
		pcScModelDirPath = (MY_CHAR*)malloc(len * sizeof(MY_CHAR));

		wsprintf(pcScModelDirPath, _T("%s%s"), acScDirPath, pcModelName);
		_wmkdir(pcScModelDirPath);
	}
	// SCS
	if (MY_STRLEN(acScsDirPath))
	{
		size_t len = MY_STRLEN(acScsDirPath) + MY_STRLEN(pcModelName) + 1;
		pcScsModelDirPath = (MY_CHAR*)malloc(len * sizeof(MY_CHAR));

		wsprintf(pcScsModelDirPath, _T("%s%s"), acScsDirPath, pcModelName);
		_wmkdir(pcScsModelDirPath);
	}
	// PRC
	if (MY_STRLEN(acPrcDirPath))
	{
		size_t len = MY_STRLEN(acPrcDirPath) + MY_STRLEN(pcModelName) + 1;
		pcPrcModelDirPath = (MY_CHAR*)malloc(len * sizeof(MY_CHAR));

		wsprintf(pcPrcModelDirPath, _T("%s%s"), acPrcDirPath, pcModelName);
		_wmkdir(pcPrcModelDirPath);
	}
#else
	// SC
	if (MY_STRLEN(acScDirPath))
	{
		size_t len = strlen(acScDirPath) + strlen(acModelNameUTF8) + 1;
		pcScModelDirPath = (MY_CHAR*)malloc(len * sizeof(MY_CHAR));

		sprintf(pcScModelDirPath, "%s%s", acScDirPath, acModelNameUTF8);
		mkdir(pcScModelDirPath, 0777);
	}
	// SCS
	if (MY_STRLEN(acScsDirPath))
	{
		size_t len = strlen(acScsDirPath) + strlen(acModelNameUTF8) + 1;
		pcScsModelDirPath = (MY_CHAR*)malloc(len * sizeof(MY_CHAR));

		sprintf(pcScsModelDirPath, "%s%s", acScsDirPath, acModelNameUTF8);
		mkdir(pcScsModelDirPath, 0777);
	}
	// PRC
	if (MY_STRLEN(acPrcDirPath))
	{
		size_t len = strlen(acPrcDirPath) + strlen(acModelNameUTF8) + 1;
		pcPrcModelDirPath = (MY_CHAR*)malloc(len * sizeof(MY_CHAR));

		sprintf(pcPrcModelDirPath, "%s%s", acPrcDirPath, acModelNameUTF8);
		mkdir(pcPrcModelDirPath, 0777);
	}
#endif

	//
	// ### TRAVERSE MODEL (create XML and export SC)
	//

	// Prepare XML file
	_ofs.open(acXmlFileName);
	_ofs << "<!--HC 23.0.0-->" << std::endl;
	_ofs << "\t<Root>" << std::endl;
	_ofs << "\t\t<ModelFile>" << std::endl;

	A3D_INITIALIZE_DATA(A3DAsmModelFileData, _sModelFileData);
	iRet = A3DAsmModelFileGet(pModelFile, &_sModelFileData);
	
	A3DTree* tree = 0;
	iRet = A3DTreeCompute(pModelFile, &tree, 0);

	A3DTreeNode* root_node = 0;
	iRet = A3DTreeGetRootNode(tree, &root_node);

	traverseTree(tree, root_node);

	A3DTreeCompute(0, &tree, 0);

	// Close XML file
	_ofs << "\t\t</ModelFile>" << std::endl;
	_ofs << "\t</Root>" << std::endl;

	_ofs.close();

	//
	// ### TERMINATE HOOPS EXCHANGE
	//
	A3DDllTerminate();

	return A3D_SUCCESS;
}
