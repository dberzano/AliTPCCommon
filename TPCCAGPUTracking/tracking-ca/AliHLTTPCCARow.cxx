// @(#) $Id$
//***************************************************************************
// This file is property of and copyright by the ALICE HLT Project          * 
// ALICE Experiment at CERN, All rights reserved.                           *
//                                                                          *
// Primary Authors: Sergey Gorbunov <sergey.gorbunov@kip.uni-heidelberg.de> *
//                  Ivan Kisel <kisel@kip.uni-heidelberg.de>                *
//                  for The ALICE HLT Project.                              *
//                                                                          *
// Permission to use, copy, modify and distribute this software and its     *
// documentation strictly for non-commercial purposes is hereby granted     *
// without fee, provided that the above copyright notice appears in all     *
// copies and that both the copyright notice and this permission notice     *
// appear in the supporting documentation. The authors make no claims       *
// about the suitability of this software for any purpose. It is            *
// provided "as is" without express or implied warranty.                    *
//***************************************************************************

#include "AliHLTTPCCARow.h"

ClassImp(AliHLTTPCCARow)

  AliHLTTPCCARow::AliHLTTPCCARow() :fHits(0),fCells(0),fCellHitPointers(0),fEndPoints(0),fNHits(0),fNCells(0),fNEndPoints(0),fX(0),fMaxY(0),fDeltaY(0),fDeltaZ(0)
{
  //* constructor
}

AliHLTTPCCARow::AliHLTTPCCARow( const AliHLTTPCCARow &)
  :fHits(0),fCells(0),fCellHitPointers(0),fEndPoints(0),fNHits(0),fNCells(0),fNEndPoints(0),fX(0),fMaxY(0),fDeltaY(0),fDeltaZ(0)
{
  //* dummy
}

AliHLTTPCCARow &AliHLTTPCCARow::operator=( const AliHLTTPCCARow &)
{
  //* dummy
  fHits = 0;
  fCells = 0;
  fCellHitPointers = 0;
  fEndPoints = 0;
  fNHits = 0;
  fNCells = 0;
  fNEndPoints = 0;
  return *this;
}

void AliHLTTPCCARow::Clear()
{
  //* clear memory
  if(fHits) delete[] fHits;
  fHits = 0;
  fCells = 0;
  fCellHitPointers = 0;    
  fEndPoints = 0;
  fNHits = fNCells = fNEndPoints = 0;
}
