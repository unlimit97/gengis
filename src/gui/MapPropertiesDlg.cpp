//=======================================================================
// Author: Donovan Parks
//
// Copyright 2009 Donovan Parks
//
// This file is part of GenGIS.
//
// GenGIS is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// GenGIS is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GenGIS.  If not, see <http://www.gnu.org/licenses/>.
//=======================================================================

#include "../core/Precompiled.hpp"

#include "../core/App.hpp"
#include "../core/MapController.hpp"
#include "../core/MapView.hpp"
#include "../core/MapTexture.hpp"
#include "../core/MapLayer.hpp"
#include "../core/Viewport.hpp"
#include "../core/LayerTreeController.hpp"
#include "../core/LocationSetLayer.hpp"
#include "../core/LocationSetController.hpp"
#include "../core/Cartogram.hpp"

#include "../gui/MapPropertiesDlg.hpp"
#include "../gui/ProgressDlg.hpp"
#include "../gui/TerrainMapWidget.hpp"

#include "../utils/StringTools.hpp"

using namespace GenGIS;

const int MAX_ELEVATION = 10000;

MapPropertiesDlg::MapPropertiesDlg(wxWindow* parent, MapLayerPtr mapLayer) :
	MapPropertiesLayout(parent),
	m_mapLayer(mapLayer),
	m_terrainMapWidget(new TerrainMapWidget(m_cboColourMap, m_scrolledWindowColourMap, &m_bColourMapChanged))
{
	SetIcon(wxIcon(App::Inst().GetExeDir() + wxT("images/CrazyEye.ico"), wxBITMAP_TYPE_ICO));

	// Limit the properties dialog to a single instance
	m_mapLayer->SetPropertiesDialog( this );
	
	m_cartogram = m_mapLayer->GetMapController()->GetMapModel()->GetCartogram();

	Init();
	Fit();
}

MapPropertiesDlg::~MapPropertiesDlg()
{
	// Tell the layer that the properties dialog is being closed
	m_mapLayer->SetPropertiesDialogOpenStatus( false );
	m_mapLayer->SetPropertiesDialog( NULL );
}

int MapPropertiesDlg::ScaleElevation(float elevation)
{
	float max = m_mapLayer->GetMapController()->GetMaxElevationGridSpace();
	float normElevation = (max-elevation) / max;

	// invert the elevation since the slider really represents an error measure
	return int(MAX_ELEVATION*normElevation);
}

float MapPropertiesDlg::ScaleElevation(int elevation)
{
	float normElevation = float(MAX_ELEVATION-elevation) / MAX_ELEVATION;

	MapControllerPtr mapController = m_mapLayer->GetMapController();
	return normElevation*mapController->GetMaxElevationGridSpace();
}

void MapPropertiesDlg::Init()
{
	// set the title of the properties dialog
	this->SetLabel( wxString(m_mapLayer->GetName().c_str()) + wxT( " : Map Properties" ) );
	
	// set state of controls on General page
	m_txtLayerName->SetValue(wxString(m_mapLayer->GetName().c_str()));
	m_txtLayerDescription->SetValue(wxString(m_mapLayer->GetDescription().c_str()));
	m_txtAuthours->SetValue(wxString(m_mapLayer->GetAuthours().c_str()));

	// set state of controls on Symbology page
	MapModelPtr mapModel = m_mapLayer->GetMapController()->GetMapModel();
	MapViewPtr mapView = m_mapLayer->GetMapController()->GetMapView();
	m_chkWireframe->SetValue(m_mapLayer->GetMapController()->GetMapView()->IsWireFrame());
	m_txtVerticalExaggeration->SetValue(wxString(StringTools::ToStringW(mapModel->GetVerticalExaggeration()).c_str()));
	m_sliderTransparency->SetValue(int(mapModel->GetTexture()->GetTransparencyPercentage()));

	m_sliderLevelOfDetail->SetMax(MAX_ELEVATION);
	m_sliderLevelOfDetail->SetMin(0);
	m_sliderLevelOfDetail->SetValue(ScaleElevation(mapView->GetEpsilon()));

	InitColourMap();

	InitCartogram();
	// set state of controls on Metadata page
	//m_txtLayerSource->SetValue(wxString(m_mapLayer->GetPath().c_str()) + _T("\\") + wxString(m_mapLayer->GetFilename().c_str()));
	m_txtLayerSource->SetValue(m_mapLayer->GetFullPath());
	InitMetaData();
}

void MapPropertiesDlg::InitColourMap()
{
	MapTexturePtr mapTexture = m_mapLayer->GetMapController()->GetMapModel()->GetTexture();

	if(mapTexture->GetColourMap())
	{
		// Populate colour map combo box with all available colour maps
		m_terrainMapWidget->SetColourMap(mapTexture->GetColourMap());
		m_terrainMapWidget->PopulateColourMapComboBox();

		// Set number of entries, interpolation, and intervals
		m_spinNumEntries->SetValue(mapTexture->GetNumColours());

		if(mapTexture->GetInterpolation() == MapTexture::DISCRETE)
			m_cboInterpolation->SetValue(_T("Discrete"));
		else if(mapTexture->GetInterpolation() == MapTexture::LINEARLY)
			m_cboInterpolation->SetValue(_T("Linearly"));

		std::vector<float> intervals;
		mapTexture->GetIntervals(intervals);
		m_terrainMapWidget->SetIntervalData(intervals);

		// Set colour map
		OnNumEntriesChange();

		if(mapTexture->GetInterpolation() == MapTexture::COLOUR_GRID)
		{
			m_cboColourMap->Enable(false);
			m_cboInterpolation->Enable(false);
			m_spinNumEntries->Enable(false);
		}
	}

	m_bColourMapChanged = false;
}

void MapPropertiesDlg::InitCartogram()
{
	m_spinAreaFudge->SetValue(_T("5"));
	m_spinValueFudge->SetValue(_T("10"));
	
	// set up location set selection
	int numSelections = App::Inst().GetLayerTreeController()->GetNumLocationSetLayers(); 
//	m_cboSelectLocation->Append(_T("None"));
	for( int i = 0; i < numSelections; i++ )
	{
		std::wstring id = App::Inst().GetLayerTreeController()->GetLocationSetLayer(i)->GetName();
		m_cboSelectLocation->Append(id.c_str());
	}
	m_cboSelectLocation->Append(_T("All"));
	m_cboSelectLocation->SetSelection(0);

	// set up vector map selection
	int numVectSelections = App::Inst().GetLayerTreeController()->GetNumVectorMapLayers(); 
	m_cboSelectVectorMap->Append(_T("None"));
	for( int i = 0; i < numVectSelections; i++ )
	{
		std::wstring id = App::Inst().GetLayerTreeController()->GetVectorMapLayer(i)->GetName();
		m_cboSelectVectorMap->Append(id.c_str());
	}
	m_cboSelectVectorMap->Append(_T("All"));
	m_cboSelectVectorMap->SetSelection(0);

	// set up measure selection
	std::vector<std::wstring> fields;
	std::vector<std::wstring> middleMan;
	bool firstPass = true;
	for( int i = 0; i < numSelections; i++ )
	{
		if (firstPass)
		{
			fields = App::Inst().GetLayerTreeController()->GetLocationSetLayer(i)->GetLocationSetController()->GetNumericMetadataFields();
			firstPass = false;
		}
		else
		{
			std::vector<std::wstring> layerFields = App::Inst().GetLayerTreeController()->GetLocationSetLayer(i)->GetLocationSetController()->GetNumericMetadataFields();
			// intersect between global fields and new fields from locationSetLayer
			sort(fields.begin(),fields.end());
			sort(layerFields.begin(),layerFields.end());
			set_intersection(fields.begin(),fields.end(),layerFields.begin(),layerFields.end(),back_inserter(middleMan));
			fields = middleMan;
			middleMan.clear();
		}
	}
	m_cboSelectMethod->Append(_T("Sequence Count"));
	for( int i = 0; i < fields.size(); i++ )
	{
		m_cboSelectMethod->Append(fields[i].c_str());
	}
	m_cboSelectMethod->SetSelection(0);
	m_spinResizePercent->Enable(false);
	m_lblResiizePercent->Enable(false);

}

void MapPropertiesDlg::OnNumEntriesChange() 
{ 
	MapTexturePtr mapTexture = m_mapLayer->GetMapController()->GetMapModel()->GetTexture();

	if(mapTexture->GetColourMap())
	{
		// Populate the wxScrolledWindow with entries.
		m_terrainMapWidget->SetEntries(
			m_scrolledWindowColourMap,
			m_spinNumEntries->GetValue(),
			m_mapLayer->GetMapController()->GetMaxElevation(), 
			m_mapLayer->GetMapController()->GetMinElevation()
		);

		m_bColourMapChanged = true;

		OnColourMapChange(*(new wxCommandEvent()));
	}
}


void MapPropertiesDlg::OnColourMapChange( wxCommandEvent& event ) 
{ 
	MapTexturePtr mapTexture = m_mapLayer->GetMapController()->GetMapModel()->GetTexture();
	if(mapTexture->GetColourMap())
	{
		m_terrainMapWidget->SetColourMap(); 
		m_bColourMapChanged = true;
	}
}

void MapPropertiesDlg::OnInterpolationChange( wxCommandEvent& event )
{
	m_bColourMapChanged = true;
	OnColourMapChange(*(new wxCommandEvent()));
}

void MapPropertiesDlg::OnEvenlySpace( wxCommandEvent& event )
{
	MapTexturePtr mapTexture = m_mapLayer->GetMapController()->GetMapModel()->GetTexture();
	if(mapTexture->GetColourMap())
	{
		m_terrainMapWidget->EvenlySpaceEntries(
			m_scrolledWindowColourMap,
			m_mapLayer->GetMapController()->GetMaxElevation(), 
			m_mapLayer->GetMapController()->GetMinElevation()
		);

		m_bColourMapChanged = true;
		OnColourMapChange(*(new wxCommandEvent()));
	}
}

void MapPropertiesDlg::InitMetaData()
{
	FileMetaData* metaData = m_mapLayer->GetMapController()->GetMetaData();
	m_txtMetaData->BeginBold();
	m_txtMetaData->AppendText(_T("Driver:\n"));
	m_txtMetaData->EndBold();
	m_txtMetaData->AppendText(wxString(StringTools::ToStringW(metaData->driverDesc).c_str()) + _T("\n"));
	m_txtMetaData->AppendText(wxString(StringTools::ToStringW(metaData->driverMetaData).c_str()) + _T("\n"));

	m_txtMetaData->BeginBold();
	m_txtMetaData->AppendText(_T("\nProjection:\n"));
	m_txtMetaData->EndBold();
	m_txtMetaData->AppendText(wxString(StringTools::ToStringW(metaData->projection).c_str()) + _T("\n"));

	m_txtMetaData->BeginBold();
	m_txtMetaData->AppendText(_T("\nDimensions:\n"));
	m_txtMetaData->EndBold();
	m_txtMetaData->AppendText(_T("X: ") + wxString(StringTools::ToStringW(metaData->xSize).c_str())
		+ _T(", Y: ") + wxString(StringTools::ToStringW(metaData->ySize).c_str())
		+ _T(", Bands: ") + wxString(StringTools::ToStringW(metaData->bands).c_str()) + _T("\n"));

	m_txtMetaData->BeginBold();
	m_txtMetaData->AppendText(_T("\nPixel Size:\n"));
	m_txtMetaData->EndBold();
	m_txtMetaData->AppendText(wxString(StringTools::ToStringW(metaData->pixelSizeX, 4).c_str()) + _T(", ") 
		+ wxString(StringTools::ToStringW(metaData->pixelSizeY, 4).c_str()) + _T("\n"));

	m_txtMetaData->BeginBold();
	m_txtMetaData->AppendText(_T("\nOrigin:\n"));
	m_txtMetaData->EndBold();
	m_txtMetaData->AppendText(wxString(StringTools::ToStringW(metaData->originX, 2).c_str()) + _T(", ") 
		+ wxString(StringTools::ToStringW(metaData->originY, 2).c_str()) + _T("\n"));

	m_txtMetaData->BeginBold();
	m_txtMetaData->AppendText(_T("\nExtents:\n"));
	m_txtMetaData->EndBold();
	m_txtMetaData->AppendText(_T("Lower, left corner: ") + wxString(StringTools::ToStringW(metaData->extents.x, 2).c_str()) + _T(", ") 
		+ wxString(StringTools::ToStringW(metaData->extents.y, 2).c_str()) + _T("\n"));
	m_txtMetaData->AppendText(_T("Upper, right corner: ") + wxString(StringTools::ToStringW(metaData->extents.dx, 2).c_str()) + _T(", ") 
		+ wxString(StringTools::ToStringW(metaData->extents.dy, 2).c_str()) + _T("\n"));

	m_txtMetaData->BeginBold();
	m_txtMetaData->AppendText(_T("\nData Type:\n"));
	m_txtMetaData->EndBold();
	m_txtMetaData->AppendText(wxString(StringTools::ToStringW(metaData->dataType).c_str()));
}

void MapPropertiesDlg::Apply()
{
	// set properties based on state of controls in General page
	m_mapLayer->SetName(m_txtLayerName->GetValue().c_str());
	m_mapLayer->SetDescription(m_txtLayerDescription->GetValue().c_str());
	m_mapLayer->SetAuthours(m_txtAuthours->GetValue().c_str());

	// set properties based on state of controls in Symbology page
	m_mapLayer->GetMapController()->GetMapView()->SetWireFrame(m_chkWireframe->IsChecked());
	m_mapLayer->GetMapController()->GetMapView()->SetVerticalExaggeration(m_mapLayer->GetMapController(), StringTools::ToDouble(m_txtVerticalExaggeration->GetValue().c_str()));

	m_mapLayer->GetMapController()->GetMapModel()->GetTexture()->SetTransparencyPercentage(m_sliderTransparency->GetValue(), this);
	m_mapLayer->GetMapController()->GetMapView()->SetEpsilon(ScaleElevation(m_sliderLevelOfDetail->GetValue()));

	ApplyColourMap();

	App::Inst().SetSaveStatus( SESSION_NOT_SAVED );
	App::Inst().GetViewport()->Refresh( false );
	App::Inst().GetLayerTreeController()->SetName(m_mapLayer, m_mapLayer->GetName());
}

void MapPropertiesDlg::ApplyColourMap()
{
	MapTexturePtr mapTexture = m_mapLayer->GetMapController()->GetMapModel()->GetTexture();

	if(mapTexture->GetColourMap() && m_bColourMapChanged)
	{
		MapTexture::INTERPOLATE interpolate;
		if(m_cboInterpolation->GetValue() == _T("Discrete"))
			interpolate = MapTexture::DISCRETE;
		else if(m_cboInterpolation->GetValue() == _T("Linearly"))
			interpolate = MapTexture::LINEARLY;

		std::vector<float> entries;
		m_terrainMapWidget->GetEntries(entries);

		mapTexture->SetColourMap(m_terrainMapWidget->GetColourMap(), entries, interpolate, this);
	}

	m_bColourMapChanged = false;
}

/** OK button event handler. */
void MapPropertiesDlg::OnOK( wxCommandEvent& event )
{
	Apply();
	Destroy();
}

void MapPropertiesDlg::OnCartogram( wxCommandEvent& event)
{
//	m_cartogram = m_mapLayer->GetMapController()->GetMapModel()->GetCartogram();
	m_cartogram->SetAreaFudge( m_spinAreaFudge->GetValue() );
	m_cartogram->SetValueFudge( m_spinValueFudge->GetValue() );
	m_cartogram->SetMeasureLabel( m_cboSelectMethod->GetStringSelection().c_str() );
	// get index, -1 for none
	int locationSetIndex = m_cboSelectLocation->GetSelection();
	int max;
	max = App::Inst().GetLayerTreeController()->GetNumLocationSetLayers();
	// if all selected
	if( locationSetIndex == max )
	{	
		int *indexes = new int[max];
		for( int i = 0; i < max; i++ )
		{
			indexes[i] = i;
		}
		m_cartogram->SetLocationSetLayer( indexes, max );
	}
	else
	{
		m_cartogram->SetLocationSetLayer( &locationSetIndex , 1 );
	}

	int vectorMapIndex = m_cboSelectVectorMap->GetSelection()-1;
	max = App::Inst().GetLayerTreeController()->GetNumVectorMapLayers(); 
	if( vectorMapIndex == max )
	{
		int *indexes = new int[max];
		for( int i = 0; i < max; i++ )
		{
			indexes[i] = i;
		}
		m_cartogram->SetVectorMap( indexes, max );
	}
	else if( vectorMapIndex >= 0 )
	{
		m_cartogram->SetVectorMap( &vectorMapIndex, 1 );
	}

	m_cartogram->MakeCartogram();
}
// Set Map, Locations and Vector back to their original positions
void MapPropertiesDlg::OnUndoCartogram( wxCommandEvent& event )
{
	m_cartogram->UndoCartogram();
}
// Toggle wether the density matrix is resized
void MapPropertiesDlg::OnResizeToggle(wxCommandEvent& event)
{
	if( m_checkResize->GetValue() ){
		m_spinResizePercent->Enable(true);
		m_lblResiizePercent->Enable(true);
		m_cartogram->SetResize(true);
	//	int resPerc = m_spinResizePercent->GetValue();
	//	m_cartogram->SetResizePercent( resPerc );
		OnSetResizePercent(event);
	}
	else
	{
		m_spinResizePercent->Enable(false);
		m_lblResiizePercent->Enable(false);
		m_cartogram->SetResize(false);
	}
}

void MapPropertiesDlg::OnSetResizePercent( wxCommandEvent& event )
{
	int resPerc = m_spinResizePercent->GetValue();
	m_cartogram->SetResizePercent( resPerc );
}
// Toggle if the value of interest is used
// as is or reversed
void MapPropertiesDlg::OnCartValueToggle(wxCommandEvent& event)
{
	// if default value
	if( m_radioOne->GetValue() )
	{
		m_cartogram->SetInvert(false);
	}
	// if inverted value
	else
	{
		m_cartogram->SetInvert(true);
	}
}
/** Apply button event handler. */
void MapPropertiesDlg::OnApply( wxCommandEvent& event )
{
	Apply();
}

/** Cancel button event handler. */
void MapPropertiesDlg::OnCancel( wxCommandEvent& event )
{
	Destroy();
}

/** Close dialog. */
void MapPropertiesDlg::OnClose( wxCloseEvent& event )
{
	Destroy();
}
