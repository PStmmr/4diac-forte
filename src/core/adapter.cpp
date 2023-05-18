/*******************************************************************************
 * Copyright (c) 2008 - 2015 ACIN, fortiss GmbH, 2018 TU Vienna/ACIN
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0.
 *
 * SPDX-License-Identifier: EPL-2.0
 *
 * Contributors:
 *    Ingo Hegny, Alois Zoitl
 *      - initial implementation and rework communication infrastructure
 *    Martin Melik-Merkumians - fixes connect, prepares for working AnyAdapter
 *******************************************************************************/
#include <string.h>
#include "adapter.h"
#ifdef FORTE_ENABLE_GENERATED_SOURCE_CPP
#include "adapter_gen.cpp"
#endif
#include "adapterconn.h"
#include "ecet.h"

CAdapter::CAdapter(CResource *pa_poSrcRes, const SFBInterfaceSpec *pa_pstInterfaceSpecSocket, const CStringDictionary::TStringId pa_nInstanceNameId, const SFBInterfaceSpec *pa_pstInterfaceSpecPlug, bool pa_bIsPlug) :
  CFunctionBlock(pa_poSrcRes, (pa_bIsPlug) ? pa_pstInterfaceSpecPlug : pa_pstInterfaceSpecSocket, pa_nInstanceNameId),
  m_nParentAdapterListEventID(0),
  m_bIsPlug(pa_bIsPlug),
  m_poPeer(nullptr),
  m_aoLocalDIs(mDIs),
  m_poAdapterConn(nullptr){
}

bool CAdapter::initialize() {
  if(!CFunctionBlock::initialize()) {
    return false;
  }
  setupEventEntryList();
  return true;
}

CAdapter::~CAdapter(){
  if (m_bIsPlug) {
    if (m_poAdapterConn != nullptr) {
      delete m_poAdapterConn;
    }
  } else {
    if (m_poAdapterConn != nullptr) {
      m_poAdapterConn->setSocket(nullptr);
    }
  }
  delete[] m_astEventEntry;
}

void CAdapter::fillEventEntryList(CFunctionBlock* paParentFB){
  for (TEventID i = 0; i < mInterfaceSpec->m_nNumEOs; ++i) {
    m_astEventEntry[i].mFB = paParentFB;
    m_astEventEntry[i].mPortId = static_cast<TPortId>(m_nParentAdapterListEventID + i);
  }
}

void CAdapter::setParentFB(CFunctionBlock *pa_poParentFB, TForteUInt8 pa_nParentAdapterlistID){
  m_nParentAdapterListEventID = static_cast<TForteUInt16>((pa_nParentAdapterlistID + 1) << 8);

  fillEventEntryList(pa_poParentFB);

  if (isPlug()) {
    //the plug is in charge of managing the adapter connection
    m_poAdapterConn = new CAdapterConnection(this, pa_nParentAdapterlistID, this);
  }
}

bool CAdapter::connect(CAdapter *pa_poPeer, CAdapterConnection *pa_poAdConn){
  bool bRetVal = false;
  if (m_poPeer == nullptr) {
    m_poPeer = pa_poPeer;
    mDIs = pa_poPeer->mDOs; //TODO: change is adapters of subtypes may be connected
    if (isSocket()) {
      m_poAdapterConn = pa_poAdConn;
    }
    bRetVal = true;
  }

  return bRetVal;
}

bool CAdapter::disconnect(CAdapterConnection *pa_poAdConn){
  if ((pa_poAdConn != nullptr) && (pa_poAdConn != m_poAdapterConn)) {
    return false; //connection requesting disconnect is not equal to established connection
  }
  m_poPeer = nullptr;
  mDIs = m_aoLocalDIs;
  if (isSocket()) {
    m_poAdapterConn = nullptr;
  }
  return true;
}

bool CAdapter::isCompatible(CAdapter *pa_poPeer) const {
  //Need to check any adapter here as we don't know which adapter side is used for checking compatibility
  return ((getFBTypeId() == pa_poPeer->getFBTypeId()) || ((getFBTypeId() == g_nStringIdANY_ADAPTER) && (g_nStringIdANY_ADAPTER != pa_poPeer->getFBTypeId())) || ((getFBTypeId() != g_nStringIdANY_ADAPTER) && (g_nStringIdANY_ADAPTER == pa_poPeer->getFBTypeId())));
}

void CAdapter::executeEvent(TEventID paEIID, CEventChainExecutionThread * const paECET){
  if (nullptr != m_poPeer) {
    if (nullptr != m_poPeer->m_astEventEntry[paEIID].mFB) {
      paECET->addEventEntry(m_poPeer->m_astEventEntry[paEIID]);
    } else {
      m_poPeer->sendOutputEvent(paEIID, paECET);
    }
  }
}

void CAdapter::setupEventEntryList(){
  m_astEventEntry = new TEventEntry[mInterfaceSpec->m_nNumEOs];
}
