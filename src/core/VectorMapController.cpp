//=======================================================================
// Author: Timothy Mankowski & Somayyeh Zangooei
//
// Copyright 2012 Timothy Mankowski & Somayyeh Zangooei
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

#include "../core/VectorMapController.hpp"

using namespace GenGIS;

template<class Archive>
void VectorMapController::serialize(Archive & ar, const unsigned int file_version)
{
	ar & m_vectorMapModel;   
	ar & m_vectorMapView; 
	ar & m_bVectorMapLoaded;
	
}
template void VectorMapController::serialize<boost::archive::text_woarchive>(boost::archive::text_woarchive& ar, const unsigned int version); 
template void VectorMapController::serialize<boost::archive::text_wiarchive>(boost::archive::text_wiarchive& ar, const unsigned int version);


/** Get OpenGL (3D) map view. */
VectorMapViewPtr VectorMapController::GetVectorMapView() const
{
	return m_vectorMapView;
}
