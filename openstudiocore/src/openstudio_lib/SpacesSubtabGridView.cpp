/**********************************************************************
*  Copyright (c) 2008-2015, Alliance for Sustainable Energy.
*  All rights reserved.
*
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Lesser General Public
*  License as published by the Free Software Foundation; either
*  version 2.1 of the License, or (at your option) any later version.
*
*  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*  Lesser General Public License for more details.
*
*  You should have received a copy of the GNU Lesser General Public
*  License along with this library; if not, write to the Free Software
*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
**********************************************************************/

#include "SpacesSubtabGridView.hpp"

#include "../shared_gui_components/OSGridView.hpp"

#include "../model/BuildingStory.hpp"
#include "../model/BuildingStory_Impl.hpp"
#include "../model/ElectricEquipment.hpp"
#include "../model/ElectricEquipment_Impl.hpp"
#include "../model/GasEquipment.hpp"
#include "../model/GasEquipment_Impl.hpp"
#include "../model/HotWaterEquipment.hpp"
#include "../model/HotWaterEquipment_Impl.hpp"
#include "../model/InteriorPartitionSurface.hpp"
#include "../model/InteriorPartitionSurface_Impl.hpp"
#include "../model/InternalMass.hpp"
#include "../model/InternalMass_Impl.hpp"
#include "../model/Lights.hpp"
#include "../model/Lights_Impl.hpp"
#include "../model/Luminaire.hpp"
#include "../model/Luminaire_Impl.hpp"
#include "../model/Model.hpp"
#include "../model/ModelObject.hpp"
#include "../model/ModelObject_Impl.hpp"
#include "../model/Model_Impl.hpp"
#include "../model/OtherEquipment.hpp"
#include "../model/OtherEquipment_Impl.hpp"
#include "../model/People.hpp"
#include "../model/People_Impl.hpp"
#include "../model/Space.hpp"
#include "../model/SpaceLoadInstance.hpp"
#include "../model/SpaceLoadInstance_Impl.hpp"
#include "../model/SpaceType.hpp"
#include "../model/SpaceType_Impl.hpp"
#include "../model/Space_Impl.hpp"
#include "../model/SteamEquipment.hpp"
#include "../model/SteamEquipment_Impl.hpp"
#include "../model/Surface.hpp"
#include "../model/Surface_Impl.hpp"
#include "../model/ThermalZone.hpp"
#include "../model/ThermalZone_Impl.hpp"

#include "../utilities/core/Assert.hpp"
#include "../utilities/idd/IddEnums.hxx"
#include "../utilities/idd/OS_Space_FieldEnums.hxx"

#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QRegExpValidator>

// FILTERS
#define STORY "Story"
#define THERMALZONE "Thermal Zone"
#define THERMALZONENAME "Thermal Zone Name"
#define SPACETYPE "Space Type"
#define SPACENAME "Space Name"
#define LOADTYPE "Load Type"
#define WINDEXPOSURE "Wind Exposure"
#define SUNEXPOSURE "Sun Exposure"
#define OUTSIDEBOUNDARYCONDITION "Outside Boundary Condition"
#define SURFACETYPE "Surface Type"
#define INTERIORPARTITIONGROUP "Interior Partition Group"

#define ALL "All"
#define UNASSIGNED "Unassigned"

// LOAD TYPES
#define SHOWALLLOADS "Show all loads"
#define INTERNALMASS "Internal Mass"
#define PEOPLE "People"
#define LIGHTS "Lights"
#define LUMINAIRE "Luminaire"
#define ELECTRICEQUIPMENT "Electric Equipment"
#define GASEQUIPMENT "Gas Equipment"
#define HOTWATEREQUIPMENT "Hot Water Equipment"
#define STEAMEQUIPMENT "Steam Equipment"
#define OTHEREQUIPMENT "Other Equipment"

namespace openstudio {

  struct ModelObjectNameSorter{
    // sort by name
    bool operator()(const model::ModelObject & lhs, const model::ModelObject & rhs){
      return (lhs.name() < rhs.name());
    }
  };

  SpacesSubtabGridView::SpacesSubtabGridView(bool isIP, const model::Model & model, QWidget * parent)
    : GridViewSubTab(isIP, model, parent)
  {
    m_spacesModelObjects = subsetCastVector<model::ModelObject>(model.getModelObjects<model::Space>());

    // Filters

    QLabel * label = nullptr;

    QVBoxLayout * layout = nullptr;

    m_filterGridLayout = new QGridLayout();
    m_filterGridLayout->setContentsMargins(7, 4, 0, 8);
    m_filterGridLayout->setSpacing(5);

    label = new QLabel();
    label->setText("Filters:");
    label->setObjectName("H2");
    m_filterGridLayout->addWidget(label, m_filterGridLayout->rowCount(), m_filterGridLayout->columnCount(), Qt::AlignTop | Qt::AlignLeft);
  }

  void SpacesSubtabGridView::showStoryFilter()
  {
    auto layout = new QVBoxLayout();

    auto label = new QLabel();
    label->setText(STORY);
    label->setObjectName("H3");
    layout->addWidget(label, Qt::AlignTop | Qt::AlignLeft);

    m_storyFilter = new QComboBox();
    initializeStoryFilter();
    m_storyFilter->setFixedWidth(OSItem::ITEM_WIDTH);
    connect(m_storyFilter, &QComboBox::currentTextChanged, this, &openstudio::SpacesSubtabGridView::storyFilterChanged);

    layout->addWidget(m_storyFilter, Qt::AlignTop | Qt::AlignLeft);
    layout->addStretch();
    m_filterGridLayout->addLayout(layout, m_filterGridLayout->rowCount() - 1, m_filterGridLayout->columnCount());
  }

  void SpacesSubtabGridView::showThermalZoneFilter()
  {
    auto layout = new QVBoxLayout();

    auto label = new QLabel();
    label->setText(THERMALZONE);
    label->setObjectName("H3");
    layout->addWidget(label, Qt::AlignTop | Qt::AlignLeft);

    m_thermalZoneFilter = new QComboBox();
    initializeThermalZoneFilter();
    m_thermalZoneFilter->setFixedWidth(OSItem::ITEM_WIDTH);
    connect(m_thermalZoneFilter, &QComboBox::currentTextChanged, this, &openstudio::SpacesSubtabGridView::thermalZoneFilterChanged);

    layout->addWidget(m_thermalZoneFilter, Qt::AlignTop | Qt::AlignLeft);
    layout->addStretch();
    m_filterGridLayout->addLayout(layout, m_filterGridLayout->rowCount() - 1, m_filterGridLayout->columnCount());
  }

  void SpacesSubtabGridView::showSpaceTypeFilter()
  {
    auto layout = new QVBoxLayout();

    auto label = new QLabel();
    label->setText(SPACETYPE);
    label->setObjectName("H3");
    layout->addWidget(label, Qt::AlignTop | Qt::AlignLeft);

    m_spaceTypeFilter = new QComboBox();
    initializeSpaceTypeFilter();
    m_spaceTypeFilter->setFixedWidth(OSItem::ITEM_WIDTH);
    connect(m_spaceTypeFilter, &QComboBox::currentTextChanged, this, &openstudio::SpacesSubtabGridView::thermalZoneFilterChanged);

    layout->addWidget(m_spaceTypeFilter, Qt::AlignTop | Qt::AlignLeft);
    layout->addStretch();
    m_filterGridLayout->addLayout(layout, m_filterGridLayout->rowCount() - 1, m_filterGridLayout->columnCount());
  }

  void SpacesSubtabGridView::showSpaceNameFilter()
  {
    auto layout = new QVBoxLayout();

    auto label = new QLabel();
    label->setText(SPACENAME);
    label->setObjectName("H3");
    layout->addWidget(label, Qt::AlignTop | Qt::AlignLeft);

    m_spaceNameFilter = new QLineEdit();
    m_spaceNameFilter->setFixedWidth(OSItem::ITEM_WIDTH);
    connect(m_spaceNameFilter, &QLineEdit::editingFinished, this, &openstudio::SpacesSubtabGridView::spaceNameFilterChanged);
    // Evan note: there are issues with using the signal below, related to the design and updating of the gridview (loss of focus, and updates per key stroke)
    //connect(m_spaceNameFilter, &QLineEdit::textEdited, this, &openstudio::SpacesSubtabGridView::spaceNameFilterChanged);

    layout->addWidget(m_spaceNameFilter, Qt::AlignTop | Qt::AlignLeft);
    layout->addStretch();
    m_filterGridLayout->addLayout(layout, m_filterGridLayout->rowCount() - 1, m_filterGridLayout->columnCount());
  }

  void SpacesSubtabGridView::showThermalZoneNameFilter()
  {
    auto layout = new QVBoxLayout();

    auto label = new QLabel();
    label->setText(THERMALZONENAME);
    label->setObjectName("H3");
    layout->addWidget(label, Qt::AlignTop | Qt::AlignLeft);

    m_thermalZoneNameFilter = new QLineEdit();
    m_thermalZoneNameFilter->setFixedWidth(OSItem::ITEM_WIDTH);
    connect(m_thermalZoneNameFilter, &QLineEdit::editingFinished, this, &openstudio::SpacesSubtabGridView::thermalZoneNameFilterChanged);

    layout->addWidget(m_thermalZoneNameFilter, Qt::AlignTop | Qt::AlignLeft);
    layout->addStretch();
    m_filterGridLayout->addLayout(layout, m_filterGridLayout->rowCount() - 1, m_filterGridLayout->columnCount());
  }

  void SpacesSubtabGridView::showWindExposureFilter()
  {
    auto layout = new QVBoxLayout();

    auto label = new QLabel();
    label->setText(WINDEXPOSURE);
    label->setObjectName("H3");
    layout->addWidget(label, Qt::AlignTop | Qt::AlignLeft);

    m_windExposureFilter = new QComboBox();
    initializeWindExposureFilter();
    m_windExposureFilter->setFixedWidth(OSItem::ITEM_WIDTH);
    connect(m_windExposureFilter, &QComboBox::currentTextChanged, this, &openstudio::SpacesSubtabGridView::windExposureFilterChanged);

    layout->addWidget(m_windExposureFilter, Qt::AlignTop | Qt::AlignLeft);
    layout->addStretch();
    m_filterGridLayout->addLayout(layout, m_filterGridLayout->rowCount() - 1, m_filterGridLayout->columnCount());
  }

  void SpacesSubtabGridView::showSunExposureFilter()
  {
    auto layout = new QVBoxLayout();

    auto label = new QLabel();
    label->setText(SUNEXPOSURE);
    label->setObjectName("H3");
    layout->addWidget(label, Qt::AlignTop | Qt::AlignLeft);

    m_sunExposureFilter = new QComboBox();
    initializeSunExposureFilter();
    m_sunExposureFilter->setFixedWidth(OSItem::ITEM_WIDTH);
    connect(m_sunExposureFilter, &QComboBox::currentTextChanged, this, &openstudio::SpacesSubtabGridView::sunExposureFilterChanged);

    layout->addWidget(m_sunExposureFilter, Qt::AlignTop | Qt::AlignLeft);
    layout->addStretch();
    m_filterGridLayout->addLayout(layout, m_filterGridLayout->rowCount() - 1, m_filterGridLayout->columnCount());
  }

  void SpacesSubtabGridView::showOutsideBoundaryConditionFilter()
  {
    auto layout = new QVBoxLayout();

    auto label = new QLabel();
    label->setText(OUTSIDEBOUNDARYCONDITION);
    label->setObjectName("H3");
    layout->addWidget(label, Qt::AlignTop | Qt::AlignLeft);

    m_outsideBoundaryConditionFilter = new QComboBox();
    initializeOutsideBoundaryConditionFilter();
    m_outsideBoundaryConditionFilter->setFixedWidth(OSItem::ITEM_WIDTH);
    connect(m_outsideBoundaryConditionFilter, &QComboBox::currentTextChanged, this, &openstudio::SpacesSubtabGridView::outsideBoundaryConditionFilterChanged);

    layout->addWidget(m_outsideBoundaryConditionFilter, Qt::AlignTop | Qt::AlignLeft);
    layout->addStretch();
    m_filterGridLayout->addLayout(layout, m_filterGridLayout->rowCount() - 1, m_filterGridLayout->columnCount());
  }

  void SpacesSubtabGridView::showSurfaceTypeFilter()
  {
    auto layout = new QVBoxLayout();

    auto label = new QLabel();
    label->setText(SURFACETYPE);
    label->setObjectName("H3");
    layout->addWidget(label, Qt::AlignTop | Qt::AlignLeft);

    m_surfaceTypeFilter = new QComboBox();
    initializeSurfaceTypeFilter();
    m_surfaceTypeFilter->setFixedWidth(OSItem::ITEM_WIDTH);
    connect(m_surfaceTypeFilter, &QComboBox::currentTextChanged, this, &openstudio::SpacesSubtabGridView::surfaceTypeFilterChanged);

    layout->addWidget(m_surfaceTypeFilter, Qt::AlignTop | Qt::AlignLeft);
    layout->addStretch();
    m_filterGridLayout->addLayout(layout, m_filterGridLayout->rowCount() - 1, m_filterGridLayout->columnCount());
  }

  void SpacesSubtabGridView::showInteriorPartitionGroupFilter()
  {
    auto layout = new QVBoxLayout();

    auto label = new QLabel();
    label->setText(INTERIORPARTITIONGROUP);
    label->setObjectName("H3");
    layout->addWidget(label, Qt::AlignTop | Qt::AlignLeft);

    m_interiorPartitionGroupFilter = new QComboBox();
    initializeInteriorPartitionGroupFilter();
    m_interiorPartitionGroupFilter->setFixedWidth(OSItem::ITEM_WIDTH);
    connect(m_interiorPartitionGroupFilter, &QComboBox::currentTextChanged, this, &openstudio::SpacesSubtabGridView::interiorPartitionGroupFilterChanged);

    layout->addWidget(m_interiorPartitionGroupFilter, Qt::AlignTop | Qt::AlignLeft);
    layout->addStretch();
    m_filterGridLayout->addLayout(layout, m_filterGridLayout->rowCount() - 1, m_filterGridLayout->columnCount());
  }

  void SpacesSubtabGridView::showLoadTypeFilter()
  {
    auto layout = new QVBoxLayout();

    auto label = new QLabel();
    label->setText(LOADTYPE);
    label->setObjectName("H3");
    layout->addWidget(label, Qt::AlignTop | Qt::AlignLeft);

    m_loadTypeFilter = new QComboBox();
    initializeLoadTypeFilter();
    m_loadTypeFilter->setFixedWidth(OSItem::ITEM_WIDTH);
    connect(m_loadTypeFilter, &QComboBox::currentTextChanged, this, &openstudio::SpacesSubtabGridView::loadTypeFilterChanged);

    layout->addWidget(m_loadTypeFilter, Qt::AlignTop | Qt::AlignLeft);
    layout->addStretch();
    m_filterGridLayout->addLayout(layout, m_filterGridLayout->rowCount() - 1, m_filterGridLayout->columnCount());
  }

  void SpacesSubtabGridView::initializeStoryFilter()
  {
    m_storyFilter->clear();
    m_storyFilter->addItem(ALL);
    m_storyFilter->addItem(UNASSIGNED);
    auto buildingStories = this->m_model.getModelObjects<model::BuildingStory>(true);
    for (auto bd : buildingStories)
    {
      QString temp("unnamed");
      if (bd.name()) {
        temp = bd.name().get().c_str();
      }
      m_storyFilter->addItem(temp, bd.handle().toString());
    }
  }

  void SpacesSubtabGridView::initializeThermalZoneFilter()
  {
    m_thermalZoneFilter->clear();
    m_thermalZoneFilter->addItem(ALL);
    m_thermalZoneFilter->addItem(UNASSIGNED);
    auto thermalZones = this->m_model.getModelObjects<model::ThermalZone>(true);
    for (auto tz : thermalZones)
    {
      QString temp("unnamed");
      if (tz.name()) {
        temp = tz.name().get().c_str();
      }
      m_thermalZoneFilter->addItem(temp, tz.handle().toString());
    }
  }

  void SpacesSubtabGridView::initializeSpaceTypeFilter()
  {
    m_spaceTypeFilter->clear();
    m_spaceTypeFilter->addItem(ALL);
    auto spacetypes = this->m_model.getModelObjects<model::SpaceType>(true);
    for (auto st : spacetypes)
    {
      QString temp("unnamed");
      if (st.name()) {
        temp = st.name().get().c_str();
      }
      m_spaceTypeFilter->addItem(temp, st.handle().toString());
    }
  }

  void SpacesSubtabGridView::initializeLoadTypeFilter()
  {

    {
      m_loadTypeFilter->addItem(SHOWALLLOADS);
    }

    {
      auto pixMap = new QPixmap(":images/mini_icons/internal_mass.png");
      OS_ASSERT(pixMap);
      m_loadTypeFilter->addItem(*pixMap, INTERNALMASS);
    }

    {
      auto pixMap = new QPixmap(":images/mini_icons/people.png");
      OS_ASSERT(pixMap);
      m_loadTypeFilter->addItem(*pixMap, PEOPLE);
    }

    {
      auto pixMap = new QPixmap(":images/mini_icons/lights.png");
      OS_ASSERT(pixMap);
      m_loadTypeFilter->addItem(*pixMap, LIGHTS);
    }

    {
      auto pixMap = new QPixmap(":images/mini_icons/luminaire.png");
      OS_ASSERT(pixMap);
      m_loadTypeFilter->addItem(*pixMap, LUMINAIRE);
    }

    {
      auto pixMap = new QPixmap(":images/mini_icons/electric_equipment.png");
      OS_ASSERT(pixMap);
      m_loadTypeFilter->addItem(*pixMap, ELECTRICEQUIPMENT);
    }

    {
      auto pixMap = new QPixmap(":images/mini_icons/gas_equipment.png");
      OS_ASSERT(pixMap);
      m_loadTypeFilter->addItem(*pixMap, GASEQUIPMENT);
    }

    {
      auto pixMap = new QPixmap(":images/mini_icons/steam_equipment.png");
      OS_ASSERT(pixMap);
      m_loadTypeFilter->addItem(*pixMap, HOTWATEREQUIPMENT);
    }

    {
      auto pixMap = new QPixmap(":images/mini_icons/steam_equipment.png");
      OS_ASSERT(pixMap);
      m_loadTypeFilter->addItem(*pixMap, STEAMEQUIPMENT);
    }

    {
      auto pixMap = new QPixmap(":images/mini_icons/other_equipment.png");
      OS_ASSERT(pixMap);
      m_loadTypeFilter->addItem(*pixMap, OTHEREQUIPMENT);
    }

  }

  void SpacesSubtabGridView::initializeWindExposureFilter()
  {
    for (auto str : model::Surface::validWindExposureValues()) {
      m_windExposureFilter->addItem(str.c_str());
    }
  }

  void SpacesSubtabGridView::initializeSunExposureFilter()
  {
    for (auto str : model::Surface::validSunExposureValues()) {
      m_sunExposureFilter->addItem(str.c_str());
    }
  }

  void SpacesSubtabGridView::initializeOutsideBoundaryConditionFilter()
  {
    for (auto str : model::Surface::validOutsideBoundaryConditionValues()) {
      m_outsideBoundaryConditionFilter->addItem(str.c_str());
    }
  }

  void SpacesSubtabGridView::initializeSurfaceTypeFilter()
  {
    for (auto str : model::Surface::validSurfaceTypeValues()) {
      m_surfaceTypeFilter->addItem(str.c_str());
    }
  }

  void SpacesSubtabGridView::initializeInteriorPartitionGroupFilter()
  {
    m_interiorPartitionGroupFilter->clear();
    m_interiorPartitionGroupFilter->addItem(ALL);
    auto interiorPartitions = this->m_model.getModelObjects<model::InteriorPartitionSurface>(true);
    for (auto ip : interiorPartitions)
    {
      QString temp("unnamed");
      if (ip.name()) {
        temp = ip.name().get().c_str();
      }
      m_interiorPartitionGroupFilter->addItem(temp, ip.handle().toString());
    }
  }

  void SpacesSubtabGridView::storyFilterChanged(const QString& text)
  {
    m_objectsFilteredByStory.clear();

    if (m_storyFilter->currentText() == ALL) {
      // nothing to filter
    }
    else if (m_storyFilter->currentText() == UNASSIGNED) {
      for (auto obj : this->m_gridController->getObjectSelector()->m_selectorObjects) {
        if (obj.cast<model::Space>().buildingStory()) {
          m_objectsFilteredByStory.insert(obj).second;
        }
      }
    }
    else {
      for (auto obj : this->m_gridController->getObjectSelector()->m_selectorObjects) {
        if (m_storyFilter->currentData().toString() != obj.handle().toString()) {
          m_objectsFilteredByStory.insert(obj).second;
        }
      }
    }

    filterChanged();
  }

  void SpacesSubtabGridView::thermalZoneFilterChanged(const QString& text)
  {
    m_objectsFilteredByThermalZone.clear();

    if (m_thermalZoneFilter->currentText() == ALL) {
      // nothing to filter
    }
    else if (m_thermalZoneFilter->currentText() == UNASSIGNED) {
      for (auto obj : this->m_gridController->getObjectSelector()->m_selectorObjects) {
        if (obj.cast<model::Space>().thermalZone()) {
          m_objectsFilteredByThermalZone.insert(obj).second;
        }
      }
    }
    else {
      for (auto obj : this->m_gridController->getObjectSelector()->m_selectorObjects) {
        if (m_thermalZoneFilter->currentData().toString() != obj.handle().toString()) {
          m_objectsFilteredByThermalZone.insert(obj).second;
        }
      }
    }
 
    filterChanged();
  }

  void SpacesSubtabGridView::spaceTypeFilterChanged(const QString& text)
  {
    m_objectsFilterdBySpaceType.clear();

    if (m_spaceTypeFilter->currentText() == ALL) {
      // nothing to filter
    }
    else if (m_spaceTypeFilter->currentText() == UNASSIGNED) {
      for (auto obj : this->m_gridController->getObjectSelector()->m_selectorObjects) {
        if (obj.cast<model::Space>().spaceType()) {
          m_objectsFilterdBySpaceType.insert(obj).second;
        }
      }
    }
    else {
      for (auto obj : this->m_gridController->getObjectSelector()->m_selectorObjects) {
        if (m_spaceTypeFilter->currentData().toString() != obj.handle().toString()) {
          m_objectsFilterdBySpaceType.insert(obj).second;
        }
      }
    }

   filterChanged();
  }
  
  void SpacesSubtabGridView::spaceNameFilterChanged()
  {
    m_objectsFilteredBySpaceName.clear();

    if (m_spaceNameFilter->text().isEmpty()) {
      // nothing to filter
    }
    else {
      for (auto obj : this->m_gridController->getObjectSelector()->m_selectorObjects) {
        QString objName(obj.name().get().c_str());
        if (!objName.contains(m_spaceNameFilter->text(), Qt::CaseInsensitive)) {
          m_objectsFilteredBySpaceName.insert(obj).second;
        }
      }
    }

    filterChanged();
  }

  void SpacesSubtabGridView::thermalZoneNameFilterChanged()
  {
    m_objectsFilteredByThermalZoneName.clear();

    if (m_thermalZoneNameFilter->text().isEmpty()) {
      // nothing to filter
    }
    else {
      for (auto obj : this->m_gridController->getObjectSelector()->m_selectorObjects) {
        QString objName(obj.name().get().c_str());
        if (!objName.contains(m_thermalZoneNameFilter->text(), Qt::CaseInsensitive)) {
          m_objectsFilteredByThermalZoneName.insert(obj).second;
        }
      }
    }

    filterChanged();
  }

  void SpacesSubtabGridView::loadTypeFilterChanged(const QString& text)
  {
    LOG(Debug, "Load filter changed: " << text);

    auto objectSelector = this->m_gridController->getObjectSelector();
    if (text == SHOWALLLOADS)
    {
      objectSelector->resetObjectFilter();
    }
    else {
      objectSelector->setObjectFilter(
        [text](const model::ModelObject &obj)->bool {
        try {
          //obj.cast<model::SpaceLoadInstance>();  TODO uncomment with correct type
          // This is a spaceloadinstance, so we want to see if it matches our filter

          if (text == INTERNALMASS)
          {
            return static_cast<bool>(obj.optionalCast<model::InternalMass>());
          }
          else if (text == PEOPLE)
          {
            return static_cast<bool>(obj.optionalCast<model::People>());
          }
          else if (text == LIGHTS)
          {
            return static_cast<bool>(obj.optionalCast<model::Lights>());
          }
          else if (text == LUMINAIRE)
          {
            return static_cast<bool>(obj.optionalCast<model::Luminaire>());
          }
          else if (text == ELECTRICEQUIPMENT)
          {
            return static_cast<bool>(obj.optionalCast<model::ElectricEquipment>());
          }
          else if (text == GASEQUIPMENT)
          {
            return static_cast<bool>(obj.optionalCast<model::GasEquipment>());
          }
          else if (text == HOTWATEREQUIPMENT)
          {
            return static_cast<bool>(obj.optionalCast<model::HotWaterEquipment>());
          }
          else if (text == STEAMEQUIPMENT)
          {
            return static_cast<bool>(obj.optionalCast<model::SteamEquipment>());
          }
          else if (text == OTHEREQUIPMENT)
          {
            return static_cast<bool>(obj.optionalCast<model::OtherEquipment>());
          }
          else
          {
            // Should never get here
            OS_ASSERT(false);
            return false;
          }
        }
        catch (...) {
          return true; // this isn't a space load instance, so don't apply filtering
        }
      }
      );
    }
  }

  void SpacesSubtabGridView::windExposureFilterChanged(const QString& text)
  {
    m_objectsFilteredByWindExposure.clear();

    for (auto obj : this->m_gridController->getObjectSelector()->m_selectorObjects) {
      if (obj.iddObjectType() == IddObjectType::OS_Surface){
        if (m_windExposureFilter->currentText() ==  obj.cast<model::Surface>().windExposure().c_str()) {
          m_objectsFilteredByWindExposure.insert(obj).second;
        }
      }
    }

    filterChanged();
  }

  void SpacesSubtabGridView::sunExposureFilterChanged(const QString& text)
  {
    m_objectsFilteredBySunExposure.clear();

    for (auto obj : this->m_gridController->getObjectSelector()->m_selectorObjects) {
      if (obj.iddObjectType() == IddObjectType::OS_Surface){
        if (m_sunExposureFilter->currentText() == obj.cast<model::Surface>().sunExposure().c_str()) {
          m_objectsFilteredBySunExposure.insert(obj).second;
        }
      }
    }
    
    filterChanged();
  }

  void SpacesSubtabGridView::outsideBoundaryConditionFilterChanged(const QString& text)
  {
    m_objectsFilteredByOutsideBoundaryCondition.clear();

    for (auto obj : this->m_gridController->getObjectSelector()->m_selectorObjects) {
      if (obj.iddObjectType() == IddObjectType::OS_Surface){
        if (m_outsideBoundaryConditionFilter->currentText() == obj.cast<model::Surface>().outsideBoundaryCondition().c_str()) {
          m_objectsFilteredByOutsideBoundaryCondition.insert(obj).second;
        }
      }
    }
    
    filterChanged();
  }

  void SpacesSubtabGridView::surfaceTypeFilterChanged(const QString& text)
  {
    m_objectsFilteredBySurfaceType.clear();

    for (auto obj : this->m_gridController->getObjectSelector()->m_selectorObjects) {
      if (obj.iddObjectType() == IddObjectType::OS_Surface){
        if (m_surfaceTypeFilter->currentText() == obj.cast<model::Surface>().surfaceType().c_str()) {
          m_objectsFilteredBySurfaceType.insert(obj).second;
        }
      }
    }
    
    filterChanged();
  }
  
  void SpacesSubtabGridView::interiorPartitionGroupFilterChanged(const QString& text)
  {
    m_objectsFilteredByInteriorPartitionGroup.clear();

    if (m_interiorPartitionGroupFilter->currentText() == ALL) {
      // nothing to filter
    }
    else {
      for (auto obj : this->m_gridController->getObjectSelector()->m_selectorObjects) {
        if (m_interiorPartitionGroupFilter->currentData().toString() != obj.handle().toString()) {
          m_objectsFilteredByInteriorPartitionGroup.insert(obj).second;
        }
      }
    }

    filterChanged();
  }

  void SpacesSubtabGridView::filterChanged()
  {
    std::set<openstudio::model::ModelObject> allFilteredObjects = m_objectsFilteredByStory;
    
    for (auto obj : m_objectsFilteredByThermalZone) {
      if (allFilteredObjects.count(obj) == 0) {
        allFilteredObjects.insert(obj).second;
      }
    }
    
    for (auto obj : m_objectsFilterdBySpaceType) {
      if (allFilteredObjects.count(obj) == 0) {
        allFilteredObjects.insert(obj).second;
      }
    }

    for (auto obj : m_objectsFilteredBySpaceName) {
      if (allFilteredObjects.count(obj) == 0) {
        allFilteredObjects.insert(obj).second;
      }
    }

    for (auto obj : m_objectsFilteredByWindExposure) {
      if (allFilteredObjects.count(obj) == 0) {
        allFilteredObjects.insert(obj).second;
      }
    }

    for (auto obj : m_objectsFilteredBySunExposure) {
      if (allFilteredObjects.count(obj) == 0) {
        allFilteredObjects.insert(obj).second;
      }
    }

    for (auto obj : m_objectsFilteredByOutsideBoundaryCondition) {
      if (allFilteredObjects.count(obj) == 0) {
        allFilteredObjects.insert(obj).second;
      }
    }

    for (auto obj : m_objectsFilteredBySurfaceType) {
      if (allFilteredObjects.count(obj) == 0) {
        allFilteredObjects.insert(obj).second;
      }
    }

    for (auto obj : m_objectsFilteredByThermalZoneName) {
      if (allFilteredObjects.count(obj) == 0) {
        allFilteredObjects.insert(obj).second;
      }
    }
    
    for (auto obj : m_objectsFilteredByInteriorPartitionGroup) {
      if (allFilteredObjects.count(obj) == 0) {
        allFilteredObjects.insert(obj).second;
      }
    }

    this->m_gridController->getObjectSelector()->m_filteredObjects = allFilteredObjects;

    this->m_gridView->requestRefreshAll();
  }

  void SpacesSubtabGridView::onDropZoneItemClicked(OSItem* item)
  {
  }

  void SpacesSubtabGridView::addObject(const IddObjectType& iddObjectType)
  {
    if (IddObjectType::OS_Space == iddObjectType.value()) {
      model::Space(this->m_model);
    }
  }

  void SpacesSubtabGridView::purgeObjects(const IddObjectType& iddObjectType)
  {
    for (auto mo : this->m_model.getConcreteModelObjects<model::Space>()){
      mo.remove();
    }
  }

} // openstudio