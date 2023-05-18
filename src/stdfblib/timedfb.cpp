/*******************************************************************************
 * Copyright (c) 2005 - 2013 ACIN, Profactor GmbH, fortiss GmbH
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0.
 *
 * SPDX-License-Identifier: EPL-2.0
 *
 * Contributors:
 *   Alois Zoitl, Gunnar Grabmair, Ingo Hegny, GErhard Ebenhofer
 *    - initial API and implementation and/or initial documentation
 *******************************************************************************/
#include "timedfb.h"
#ifdef FORTE_ENABLE_GENERATED_SOURCE_CPP
#include "timedfb_gen.cpp"
#endif

const CStringDictionary::TStringId CTimedFB::scm_aunEINameIds[] = {g_nStringIdSTART, g_nStringIdSTOP};
const TDataIOID CTimedFB::scm_anEIWith[] = {0, scmWithListDelimiter};
const TForteInt16 CTimedFB::scm_anEIWithIndexes[] = {0, -1};
const CStringDictionary::TStringId CTimedFB::scm_aunEONameIds[] = {g_nStringIdEO};

const CStringDictionary::TStringId CTimedFB::scm_aunDINameIds[] = {g_nStringIdDT};
const CStringDictionary::TStringId CTimedFB::scm_aunDIDataTypeNameIds[] = {g_nStringIdTIME};

const SFBInterfaceSpec CTimedFB::scm_stFBInterfaceSpec = {
  2, scm_aunEINameIds, scm_anEIWith, scm_anEIWithIndexes,
  1, scm_aunEONameIds, nullptr, nullptr,
  1, scm_aunDINameIds, scm_aunDIDataTypeNameIds,
  0, nullptr, nullptr,
  0, nullptr,
  0, nullptr
};

CTimedFB::CTimedFB(const CStringDictionary::TStringId paInstanceNameId, CResource *paSrcRes, ETimerActivationType paType) :
      CEventSourceFB( paSrcRes, &scm_stFBInterfaceSpec, paInstanceNameId){
  setEventChainExecutor(paSrcRes->getResourceEventExecution());
  mActive = false;
  mTimeListEntry.mTimeOut = 0;
  mTimeListEntry.mInterval = 0;
  mTimeListEntry.mNext = nullptr;
  mTimeListEntry.mType = paType;
  mTimeListEntry.mTimedFB = this;
}

void CTimedFB::executeEvent(TEventID paEIID, CEventChainExecutionThread * const ){
  switch(paEIID){
    case cg_nExternalEventID:
      sendOutputEvent(csm_nEOID, getEventChainExecutor());
      break;
    case csm_nEventSTOPID:
      if(mActive){
        getTimer().unregisterTimedFB(this);
        mActive = false;
      }
      break;
    case csm_nEventSTARTID:
      if(!mActive){
        getTimer().registerTimedFB( &mTimeListEntry, DT());
        mActive = true;
      }
      break;
    default:
      break;
  }
}

EMGMResponse CTimedFB::changeFBExecutionState(EMGMCommandType pa_unCommand){
  EMGMResponse eRetVal = CFunctionBlock::changeFBExecutionState(pa_unCommand);
  if((EMGMResponse::Ready == eRetVal) && ((EMGMCommandType::Stop == pa_unCommand) || (EMGMCommandType::Kill == pa_unCommand)) && mActive) {
    getTimer().unregisterTimedFB(this);
    mActive = false;
  }
  return eRetVal;
}
