/* Buzz Machine Loader
 * Copyright (C) 2006 Buzztrax team <buzztrax-devel@buzztrax.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef BUZZ_MACHINE_CALLBACKS_PRE12_H
#define BUZZ_MACHINE_CALLBACKS_PRE12_H

#include "CMICallbacks.h"
#include "mdkimp.h"

// should be MAX_BUFFER_LENGTH=256
//#define BMC_AUXBUFFER_SIZE MAX_BUFFER_LENGTH
#define BMC_AUXBUFFER_SIZE 1024

typedef struct _CHostCallbacks CHostCallbacks;

struct CMachineInfo;
class CMachineInterface;

class BuzzMachineCallbacksPre12 : public CMICallbacks
{
public:
    BuzzMachineCallbacksPre12(CMachine *_machine, CMachineInterface *_machine_interface,CMachineInfo *_machine_info,CHostCallbacks **_host_callbacks) {
        machine=_machine;
        machine_interface=_machine_interface;
        machine_info=_machine_info;
        defaultWaveLevel.LoopEnd=0;
        defaultWaveLevel.LoopStart=0;
        defaultWaveLevel.numSamples=0;
        defaultWaveLevel.pSamples=NULL;
        defaultWaveLevel.RootNote=0;
        defaultWaveLevel.SamplesPerSec=0;
        mdkHelper=NULL;
        host_callbacks=_host_callbacks;
    }
    virtual ~BuzzMachineCallbacksPre12() {}

    CWaveInfo const *GetWave(int const i);
    CWaveLevel const *GetWaveLevel(int const i, int const level);
    void MessageBox(char const *txt);
    void Lock();
    void Unlock();
    int GetWritePos();
    int GetPlayPos();
    float *GetAuxBuffer();
    void ClearAuxBuffer();
    int GetFreeWave();
    bool AllocateWave(int const i, int const size, char const *name);
    void ScheduleEvent(int const time, dword const data);
    void MidiOut(int const dev, dword const data);
    short const *GetOscillatorTable(int const waveform);

    // envelopes
    int GetEnvSize(int const wave, int const env);
    bool GetEnvPoint(int const wave, int const env, int const i, word &x, word &y, int &flags);

    CWaveLevel const *GetNearestWaveLevel(int const i, int const note);

    // pattern editing
    void SetNumberOfTracks(int const n);
    CPattern *CreatePattern(char const *name, int const length);
    CPattern *GetPattern(int const index);
    char const *GetPatternName(CPattern *ppat);
    void RenamePattern(char const *oldname, char const *newname);
    void DeletePattern(CPattern *ppat);
    int GetPatternData(CPattern *ppat, int const row, int const group, int const track, int const field);
    void SetPatternData(CPattern *ppat, int const row, int const group, int const track, int const field, int const value);

    // sequence editing
    CSequence *CreateSequence();
    void DeleteSequence(CSequence *pseq);

    // special ppat values for GetSequenceData and SetSequenceData
    // empty = NULL
    // <break> = (CPattern *)1
    // <mute> = (CPattern *)2
    // <thru> = (CPattern *)3
    CPattern *GetSequenceData(int const row);
    void SetSequenceData(int const row, CPattern *ppat);
private:
    CMachine *machine;
    CMachineInterface *machine_interface;
    CMachineInfo *machine_info;
    CWaveLevel defaultWaveLevel;/*={0,NULL,0,0,0,0};*/
    CMDKImplementation *mdkHelper;
    CHostCallbacks **host_callbacks;
    // static float ... does not work?
    float auxBuffer[2*BMC_AUXBUFFER_SIZE]; // gah, inefficient, because BuzzMachineCallbacks objects could all share same aux buffer
};

#endif // BUZZ_MACHINE_CALLBACKS_H
