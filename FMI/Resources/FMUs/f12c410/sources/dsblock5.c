/* Begin dsblock5.c */
/* File version: 1.4, 1998-03-20 */

/*
 * Copyright (C) 1997-2001 Dynasim AB.
 * All rights reserved.
 *
 */

}



#undef externalTable_
#ifdef NBR_TASKS
DYMOLA_STATIC int nbrTasks_=NBR_TASKS;
#else
DYMOLA_STATIC int nbrTasks_=0;
#endif

#if !defined(DYMOLA_DSPACE) 
DYMOLA_STATIC double
DymolaStartTimers_[
#ifdef NrDymolaTimers_
NrDymolaTimers_ ? NrDymolaTimers_ : 1
#else
	1
#endif
];
DYMOLA_STATIC double DymolaTimeZero[100000]={0};
DYMOLA_STATIC int DymolaTimeZeroLength=100000;
#endif

#if !defined(DymolaHaveUpdateInitVars)
DYMOLA_STATIC void UpdateInitVars(double *time, double X_[], double XD_[], double U_[], \
      double DP_[], int IP_[], Dymola_bool LP_[], double F_[], double Y_[], double W_[], double QZ_[], double duser_[], int iuser_[], void*cuser_[], struct DYNInstanceData*did_,int initialCall) 
{
  return;
}
#endif



	  /* Must be initialized (and thus defined) because moutil is included first*/
	  static int DYNStrInit(struct DYNInstanceData*did_) {
		  if (DYNX(DYNAuxStr_,0)==0) {
			  int j;
			  for(j=0;j<sizeof(DYNAuxStr_)/sizeof(*DYNAuxStr_);++j) DYNAuxStr_[j]=did_->DYNAuxStrBuff_vec+j*
#if defined(MAXAuxStrLen_) && MAXAuxStrLen_>10
				  MAXAuxStrLen_
#else
				  10
#endif
				  ;
		  }
		  return 0;
	  }

	  DYMOLA_STATIC void DYNStrClear(struct DYNInstanceData* did_) {
		  memset(DYNAuxStr_, 0, sizeof(DYNAuxStr_));
	  }

	  DYMOLA_STATIC void DYNSetAuxStringD(struct DYNInstanceData*did_,const char*s,int i, int setDEventIfDifferent) {
		  DYNStrInit(did_);
		  if (i>=0 && i<sizeof(DYNAuxStr_)/sizeof(*DYNAuxStr_)) {
			  int j,mlen=
#if defined(MAXAuxStrLen_) && MAXAuxStrLen_>10
			MAXAuxStrLen_
#else
		  10
#endif
		      ;
			  if (setDEventIfDifferent && !did_->AnyDEvent_var && strncmp(&(DYNAuxStr_[i][0]), s, mlen-1) != 0) did_->AnyDEvent_var = 1;
			  for(j=0;j<mlen-1 && s[j];++j) DYNAuxStr_[i][j]=s[j];
			  DYNAuxStr_[i][j]=0;
			  if (s[j]) {DymosimMessage("Truncated string variable to");DymosimMessage(DYNAuxStr_[i]);}
		  } else DymosimMessage("Internal error in String handling.");
	  }
	  DYMOLA_STATIC void DYNSetAuxString(struct DYNInstanceData*did_, const char*s, int i) {
		  DYNSetAuxStringD(did_, s, i, 0);
	  }
	  DYMOLA_STATIC void DYNSetAuxStringArrayD(struct DYNInstanceData*did_, struct StringArray s, int i, int setDEventIfDifferent) {
		  int nrElem, j;
		  nrElem = StringNrElements(s);
		  if (i >= 0 && i + nrElem <= sizeof(DYNAuxStr_) / sizeof(*DYNAuxStr_)) {
			  for (j = 0; j<nrElem; ++j) {
				  DYNSetAuxStringD(did_, s.data[j], i + j, setDEventIfDifferent);
			  }
		  } else DymosimMessage("Internal error in String array handling.");
	  }
	  DYMOLA_STATIC void DYNSetAuxStringArray(struct DYNInstanceData*did_,struct StringArray s,int i) {
		  DYNSetAuxStringArrayD(did_, s, i, 0);
	  }
	  DYMOLA_STATIC const char*DYNGetAuxStr(struct DYNInstanceData*did_,int i) {
		  DYNStrInit(did_);
		  if (i>=0 && i<sizeof(DYNAuxStr_)/sizeof(*DYNAuxStr_)) {
			  return DYNAuxStr_[i];
		  }
		  return "";
	  }
	  DYMOLA_STATIC size_t DYNGetMaxAuxStrLen() {
		  return
#if defined(MAXAuxStrLen_) && MAXAuxStrLen_>10
			  MAXAuxStrLen_
#else
			  10
#endif
			  ;

	  }
	  static int QNLfunc_vec[QNLmax_ ? QNLmax_ : 1] = {0};
	  DYMOLA_STATIC int* QNLfunc = QNLfunc_vec;
	  static int QNLjac_vec[QNLmax_ ? QNLmax_ :1] = {0};
	  DYMOLA_STATIC int* QNLjac = QNLjac_vec;
	  DYMOLA_STATIC int QNLmax=QNLmax_;
	  static int QNLcalls_vec[QNLmax_ ? QNLmax_ : 1] = { 0 };
	  DYMOLA_STATIC int* QNLcalls = QNLcalls_vec;
#if !defined(NExternalObject_)
#define NExternalObject_ 10
#endif
	  
	  DYMOLA_STATIC int Buffersize = 20000;
	  DYMOLA_STATIC int setDefault_=0;
	  DYMOLA_STATIC int setDefaultX_=0,setDefaultU_=0,setDefaultY_=0,setDefaultP_=0,setDefaultDX_=0,setDefaultW_=0;
	  DYMOLA_STATIC LIBDS_API_AFTER void freeDelay(delayStruct* del, const size_t nbrDel);

	  DYMOLA_STATIC size_t getNbrExternalObjects() {
#if defined(NExternalObject_)
		  return (size_t) NExternalObject_;
#else
		  return 0;
#endif
	  }


DYMOLA_STATIC void delayBuffersCloseNew(struct DYNInstanceData*did_) {
	 int i;
	 for(i=0;i<SizeDelay_;++i) delayID_[i]=0;
	 for(i=0;i<MAXAux_;++i) Aux_[i]=0;
	 for(i=0;i<SizePre_;++i) QPre_[i]=0;
	 for(i=0;i<SizePre_;++i) RefPre_[i]=0;
	 for(i=0;i<SizeEq_;++i) EqRemember2_[i]=EqRemember1_[i]=EqRememberTemp_[i]=0;
	 for(i=0;i<NWhen_;++i) QEvaluateNew_[i]=QEvaluate_[i]=0;
#ifdef DYNLargeGlobalHelp
	 if (did_->helpvar_vec)
#endif
	 for(i=0;i<NGlobalHelp_;++i) DYNhelp[i]=0;
	 for(i=0;i<NGlobalHelpI_;++i) did_->helpvari_vec[i]=0;
	 for(i=0;i<2*NRel_+1;++i) 
      oldQZ2_[i]=oldQZ3_[i] =  QZold_[i]=oldQZ_[i]=oldQZDummy_[i]=0;
	 for(i=0;i<NRel_+1;++i) QRel_[i]=QM_[i]=Qn_[i] = Qp_[i]=Qscaled_[i]=0.0;
	 for(i=0;i<NSamp_;++i) {NextSampleTime_[i]=NextSampleTimeNew_[i]=0;NextSampleAct_[i]=NextSampleActNew_[i]=0;}
	 for(i=0;i<NRel_+1;++i) QL_[i]=Qenable_[i]=0;
	 for(i=0;i<NTim_+1;++i) QTimed_[i]=0;
	 EqRemember1Time_=-1e33;
	 EqRemember2Time_=-1e33;
	 EqRemAcc1Time_ = -1e33;
	 EqRemAcc2Time_ = -1e33;
	 EqRemTempTime_ = -1e33;
	 EqRemTempTimeAcc_ = -1e33;
	 (did_->hasStoredInTemp) = 0;
	 (did_->eqRememberReplaceOldDynamics) = 0;
	 (did_->eqRememberReplaceOldAccepted) = 0;
	 (did_->decoupleTime_var)=-1e33;
	 if (NExternalObject_ > 0) {
#ifdef DYNCallStackData
		 struct DYNFunctionData_ dynFuncData, * DYNStackData_;
		 DYNPropagateDidToThread(did_);
		 /* After propagate */
		 dynFuncData = DYNGetFunctionData();
		 DYNStackData_ = &dynFuncData;
#endif
		 if (setjmp(DYNPCSDGlobal_jmp_buf.TheBuffer) == 0) {
			 for (i = NExternalObject_ - 1; i >= 0; --i) {
				 /* Reverse order in case of dependencies */
				 void* x = did_->externalTable_vec[i].obj_;
				 did_->externalTable_vec[i].obj_ = 0;
				 if (x && did_->externalTable_vec[i].destructor_)
					 (*(did_->externalTable_vec[i].destructor_))(x);
				 did_->externalTable_vec[i].destructor_ = 0;
#if (defined(_OPENMP) && !defined(DISABLE_DYMOLA_OPENMP))
				 if (did_->externalTable_vec[i].haveLock_) {
					 omp_destroy_lock(&(did_->externalTable_vec[i].lockExternal_));
					 did_->externalTable_vec[i].haveLock_ = 0;
				 }
#endif
			 }
		 } else {
			 DymosimMessage("Some destructor caused an error. That is not good.\n");
		 }
	 }

	 for(i=0;i<did_->DymolaTimerStructsLen_var;i++) {
		 did_->DymolaTimerStructs_vec[i].num=0;
	 }
	 freeDelay(did_->del_vec, SizeDelay_ ? SizeDelay_ : 1);
#if (defined(_OPENMP) && !defined(DISABLE_DYMOLA_OPENMP)) && defined(_MSC_VER)
	 if (!getenv("OMP_WAIT_POLICY")) Sleep(200); 
	 /* Otherwise unloading the DLL may crash, see also https://stackoverflow.com/questions/34439956/vc-crash-when-freeing-a-dll-built-with-openmp */
#endif
#ifdef DYNLargeGlobalHelp
	 if (did_->helpvar_vec) free(did_->helpvar_vec);
	 did_->helpvar_vec = 0;
#endif
#if defined(NSparse_) && NSparse_ && (defined(DYMOSIM) || (defined(BUILDFMU) && !defined(FMU_SOURCE_CODE_EXPORT))) && !defined(LIBDS_DLL) && !defined(DYMOLA_AS_MKMK_MODULE)
	 for (i = 0; i < NSparse_; i++) {
		 if (did_->sparse_data[i])
			 DynSuperLUMTFree(did_->sparse_data[i]);
		 did_->sparse_data[i] = NULL;
	 }
#endif
}
DYMOLA_STATIC void delayBuffersClearExternalTables(struct DYNInstanceData*did_) {
	int i;
	for (i = 0; i < NExternalObject_; i++) {
		did_->externalTable_vec[i].obj_ = 0;
		did_->externalTable_vec[i].destructor_ = 0;
	}
}
DYMOLA_STATIC void delayBuffersClose(void) {
	delayBuffersCloseNew(&tempData);
}
DYMOLA_STATIC int dynInstanceDataSize() {
	return sizeof(struct DYNInstanceData);
}

DYMOLA_STATIC size_t externalTableSize() {
	return sizeof(struct ExternalTable_);
}

DYMOLA_STATIC void* dynExternalObjectFirst(struct DYNInstanceData* did) {
	if (did == 0) {
		return 0;
	}
	return did->externalTable_vec[0].obj_;
}

DYMOLA_STATIC void CheckForEvents(struct DYNInstanceData*did_,double Time, int Init, int Event, 
       double QZ_[], int nrel_, double F_[], int nx_,double*duser_,int*iuser_)
/* SCRAMBLE ON */
{
#define DebugCheckForEvents 0
 

#define OvershootFactor 1.2
 	/* */
#define FindLastEvent 1
 	/* */
#define CheckForEventsEps 1e-10
 	/* */ 
#define SecondDegreeOvershootFactor 1.04

#define SecondDegreeUncertainty 0.4


#define SecondDegreeUncertainty2 0.7
	int ZZZ39;
    int ZZZ715;   static double oldTime,oldDummyTime=-1e30;
#if SecondDegree
	static double oldDummyTime2;
#endif
   static double oldstepSizeRatio;
   static double c1, c2, c1start;   static double T1end=-1e30;   static double T2end=-1e30, stepSizeRatio=1;   double ZZZ8329, ZZZ7652;

#ifdef InterpolateStatesForInline
   static const double CheckForEventsMinStep=0.2;
#else
   static const double CheckForEventsMinStep=0;
#endif
   int ZZZ5998;    if (Init) {
#if FindEvent_
     DymosimMessage("");
     DymosimMessage("Approximative event finder used. Must be used with Euler method.");     DymosimMessage("");
#endif
     StepSize = 0;     LastTime = -1E30;     T1end = -1E30;     T2end = -1E30; 	oldTime = Time; 	oldDummyTime = -1e30;
#if SecondDegree
 	oldDummyTime2 = Time; oldstepSizeRatio = 1.0;
#endif
 	c1=1; 	c2=1; 	c1start=1; 	stepSizeRatio=1;   }   if (StepSize == 0 && Time > LastTime && LastTime>-1e29)     StepSize = Time - LastTime; 
   if (Event)      LastTime = Time;    ZZZ5998 = Time>oldDummyTime;   if (ZZZ5998) 	{
#if SecondDegree
 	  oldDummyTime2=oldDummyTime; 	  oldstepSizeRatio=stepSizeRatio;
 	  if (StepSize!=0) { 		  if (Time>=T1end && Time<T2end) 			  stepSizeRatio = (T1end-oldTime)/StepSize; 		  else 			  stepSizeRatio = (Time-oldTime)/StepSize; 	  } 	  for (ZZZ715 = 0; ZZZ715 < 2*nrel_;ZZZ715++) {oldQZ3_[ZZZ715]=oldQZ2_[ZZZ715];oldQZ2_[ZZZ715]=oldQZ_[ZZZ715];}
#endif
 	  for(ZZZ715=0;ZZZ715<2*nrel_;ZZZ715++) oldQZ_[ZZZ715]=oldQZDummy_[ZZZ715];   }	
   { 	  for(ZZZ715=0;ZZZ715<2*nrel_;ZZZ715++) oldQZDummy_[ZZZ715]=Qenable_[ZZZ715/2+1] ? QZ_[ZZZ715] : 0;   }    if (StepSize!=0 && ZZZ5998) { 	  
#if DebugCheckForEvents
	   double ZZZ1317 = 0;  for (ZZZ715 = 0; ZZZ715 < 2*nrel_; ZZZ715++)  { 		  if (Qenable_[ZZZ715/2+1]) { 			  if (oldQZ_[ZZZ715]*QZ_[ZZZ715]<0) {
 				  double ZZZ8860; 				  ZZZ8860=QZ_[ZZZ715]/(QZ_[ZZZ715]-oldQZ_[ZZZ715]); 				  if (ZZZ8860>ZZZ1317) {ZZZ1317=ZZZ8860;ZZZ39=ZZZ715/2;} 			  } 		  } 	  } 	  if (ZZZ1317>0) { 		  char ZZZ732[200]; 		  if (Time<T2end) { 			  sprintf(ZZZ732,"Event at projected time %.10g overshoot %.10g",T1end,c1*ZZZ1317+1);
 		  } else if (stepSizeRatio>1+CheckForEventsEps || stepSizeRatio<1-CheckForEventsEps) { 			  sprintf(ZZZ732,"Missed event at time %.10g interpolated at %.10g",Time,Time-ZZZ1317*stepSizeRatio*StepSize); 		  } else { 			  sprintf(ZZZ732,"Event at time %.10g interpolated at %.10g",Time,Time-ZZZ1317*StepSize); 		  } 		  DymosimMessage(ZZZ732);
#if SecondDegree
 		  sprintf(ZZZ732,"Relation %d QZ=%.10g %.10g oldQZ=%.10g oldQZ2=%.10g oldQZ3=%.10g",ZZZ39,QZ_[2*ZZZ39],QZ_[2*ZZZ39+1],oldQZ_[2*ZZZ39],oldQZ2_[2*ZZZ39],oldQZ3_[2*ZZZ39]);
#else
 		  sprintf(ZZZ732,"Relation %d QZ=%.10g %.10g oldQZ=%.10g",ZZZ39,QZ_[2*ZZZ39],QZ_[2*ZZZ39+1],oldQZ_[2*ZZZ39]);

#endif
 		  DymosimMessage(ZZZ732); 	  }
#endif
 	     }    if (StepSize != 0 && Time >= T2end) {     c1 = c1start = FindLastEvent ? 0 :2; 	ZZZ7652 = (NextTimeEvent-Time)/StepSize; /* */
 	ZZZ39=-1; 	
	if (ZZZ7652>0 && ZZZ7652<2 && (FindLastEvent ? ZZZ7652>c1: ZZZ7652<c1)) { 		c1=ZZZ7652; 	}     for (ZZZ715 = 0; ZZZ715 < 2*nrel_; ZZZ715++) {        ZZZ8329 = (QZ_[ZZZ715] - oldQZ_[ZZZ715])/stepSizeRatio;       if (QZ_[ZZZ715] * (OvershootFactor*ZZZ8329*2 + QZ_[ZZZ715]) < 0 && Qenable_[ZZZ715/2+1] && QZ_[2*(ZZZ715/2)]*QZ_[2*(ZZZ715/2)+1]>0 ) {         /* */         ZZZ7652 = -QZ_[ZZZ715]/ZZZ8329+(OvershootFactor-1);  /* */
#if SecondDegree
 		if (oldDummyTime2>-1e30 && (oldQZ_[ZZZ715]>0 ? oldQZ2_[ZZZ715]>oldQZ_[ZZZ715] : oldQZ2_[ZZZ715]<oldQZ_[ZZZ715])) { 			/* */ 			double ZZZ3419, ZZZ8687, ZZZ4213, ZZZ2231, ZZZ4006, ZZZ6591, ZZZ5281, ZZZ8430, ZZZ7134; 			ZZZ3419=QZ_[ZZZ715]; 			ZZZ8687=(stepSizeRatio+oldstepSizeRatio); 			ZZZ4213=stepSizeRatio*ZZZ8687*oldstepSizeRatio; 			ZZZ2231=(ZZZ8687*ZZZ8687*(oldQZ_[ZZZ715]-ZZZ3419)-stepSizeRatio*stepSizeRatio*(oldQZ2_[ZZZ715]-ZZZ3419))/ZZZ4213; 			ZZZ4006=(-ZZZ8687*(oldQZ_[ZZZ715]-ZZZ3419)+stepSizeRatio*(oldQZ2_[ZZZ715]-ZZZ3419))/ZZZ4213; 			ZZZ6591=4*ZZZ3419*ZZZ4006; 			ZZZ5281=ZZZ2231*ZZZ2231;
 			ZZZ8430=(oldQZ_[ZZZ715]>0 ? oldQZ3_[ZZZ715]>oldQZ2_[ZZZ715] : oldQZ3_[ZZZ715]<oldQZ2_[ZZZ715])  				? SecondDegreeUncertainty : SecondDegreeUncertainty2; 			ZZZ7134=ZZZ5281-(ZZZ6591>0 ? ZZZ6591*(1+ZZZ8430) : ZZZ6591*(1-ZZZ8430)); 			if (ZZZ7134>=0) { 			double ZZZ5803; 			ZZZ5803=-(2*ZZZ3419/(-ZZZ2231-(ZZZ2231>0?1:-1)*sqrt(ZZZ7134)))+(SecondDegreeOvershootFactor-1);
#if DebugCheckForEvents
 			{char ZZZ732[200]; 			sprintf(ZZZ732,"%d ZZZ3419 %.10g ZZZ8687 %.10g ZZZ4213 %.10g ZZZ2231 %.10g ZZZ7652 %.10g ZZZ7134 %.10g",ZZZ715,ZZZ3419,ZZZ8687,ZZZ4213,ZZZ2231,ZZZ4006,ZZZ7134); 			DymosimMessage(ZZZ732);
 			sprintf(ZZZ732,"C1: %g C2: %g beta=-%g alpha=-%g QZ=%.10g oldQZ=%.10g oldQZ2=%.10g",ZZZ7652,ZZZ5803,stepSizeRatio+oldstepSizeRatio,stepSizeRatio, 				QZ_[ZZZ715],oldQZ_[ZZZ715],oldQZ2_[ZZZ715]); 			DymosimMessage(ZZZ732); 			}
#endif
 			if (ZZZ5803>-0.5 && ZZZ5803 < 2.5) ZZZ7652=ZZZ5803; 			} 		}
#endif
         if (ZZZ7652 > 0 && ZZZ7652<2 && (FindLastEvent ? ZZZ7652>c1: ZZZ7652<c1)) {       /* */
           c1 = ZZZ7652;      /* */ZZZ39=ZZZ715/2;
		 }       }     }      if (c1 != 1E30 && c1 != c1start && (1+(ZZZ39<0?CheckForEventsEps:0))*c1<1+CheckForEventsMinStep) {       /* */ 	  if (c1<CheckForEventsMinStep) c1=CheckForEventsMinStep;       c2 = 2 - c1;
       T1end = Time + (1-CheckForEventsEps)*StepSize;            T2end = Time + (2-CheckForEventsEps)*StepSize; /* */
#if DebugCheckForEvents
 	  {char ZZZ732[200]; 	  sprintf(ZZZ732,"Project at %.10g to %.10g Short %.10g Long %.10g",Time,T1end,c1*StepSize,c2*StepSize); 	  DymosimMessage(ZZZ732);} 	  {char ZZZ732[200];
#if SecondDegree
 	  sprintf(ZZZ732,"Relation %d QZ=%.10g %.10g oldQZ=%.10g oldQZ2=%.10g oldQZ3=%.10g",ZZZ39,QZ_[2*ZZZ39],QZ_[2*ZZZ39+1],oldQZ_[2*ZZZ39],oldQZ2_[2*ZZZ39],oldQZ3_[2*ZZZ39]);
#else
 	  sprintf(ZZZ732,"Relation %d QZ=%.10g %.10g oldQZ=%.10g",ZZZ39,QZ_[2*ZZZ39],QZ_[2*ZZZ39+1],oldQZ_[2*ZZZ39]);
#endif
 	  DymosimMessage(ZZZ732); 	  }
#endif
 	} else { 		c2=1; 		T2end=Time; 		T1end=Time; 	}
   }   oldTime=oldDummyTime=Time;    currentStepSize_ = StepSize;
#ifdef InterpolateStatesForInline
   currentStepSizeRatio_ = 1;
#endif
   currentStepSizeRatio2_ = 1;    if (Time < T1end) {
 	currentStepSizeRatio2_ = c1;
#ifdef InterpolateStatesForInline
 	currentStepSizeRatio_ = c1;
#else
     currentStepSize_ = c1*StepSize;
#endif
     for (ZZZ715 = 0; ZZZ715 < nx_; ZZZ715++) {       F_[ZZZ715] = F_[ZZZ715]*c1;     }     /* */
   } else if (Time < T2end) { 	currentStepSizeRatio2_ = c2;
#ifdef InterpolateStatesForInline
 	currentStepSizeRatio_ = c2;
#else
     currentStepSize_ = c2*StepSize;
#endif
     for (ZZZ715 = 0; ZZZ715 < nx_; ZZZ715++) {       F_[ZZZ715] = F_[ZZZ715]*c2;     }
     /* */ 	oldTime=T1end;   }}

/* SCRAMBLE OFF */

DYMOLA_STATIC int sprintfC(char*s, const char*format, ...);

DYMOLA_STATIC Dymola_bool sampleFunction(struct DYNInstanceData*did_,double Time, double start, double interval, int counter,
                      Dymola_bool Init, Dymola_bool Event) {
  struct BasicDDymosimStruct*basicD=getBasicDDymosimStruct();
  Dymola_bool samp = false;

  if (Init  || (Event && NextSampleAct_[counter]==0)) {
	double x;
    basicD->mOrigTimeError=Dymola_max(basicD->mOrigTimeError,fabs(start)); /* Collect them */

    x=findCounter(Dymola_max(Time,start),start,interval);
	if (Init || x>NextSampleTime_[counter])
		NextSampleTime_[counter]=x;
    /* Samples at start,start+interval,...*/
    /* Replace Dymola_max(Time,start) by Time to sample at ...,start-interval,start,start+interval */
  };

  if (Event) {
    double eventTime=start+(NextSampleTime_[counter]-1)*interval;
    const double eventAccuracy=
#ifndef DynSimStruct
		5e-14
#else 
		1e-7
#endif
		*(fabs(Time)+basicD->mOrigTimeError);
    /* 5*eps to guard against different times */
      /*DymosimMessageDouble("Event at time: ",Time);*/
      /*DymosimMessageDouble("Event Time:",eventTime);*/
    while (eventTime<=Time+eventAccuracy) {
      NextSampleTime_[counter]+=1;
      eventTime=start+(NextSampleTime_[counter]-1)*interval;
      /*DymosimMessageDouble("Sampling at time: ", Time);*/
      /*DymosimMessageDouble("Next sampling",eventTime);*/
      samp = true;
	}
	NextSampleTimeNew_[counter]=NextSampleTime_[counter];
	NextSampleActNew_[counter]=1;
    registerTimeEventNew(eventTime, basicD); /* The next event for this sampler */

	samp = samp && (Iter == 1);
	{
		struct BasicIDymosimStruct* basicI = getBasicIDymosimStruct();
		if (samp && ((!FirstEvent && basicI->mPrintEvent&(1 << 1)) || (FirstEvent && basicI->mPrintEvent&(1 << 2)))) {
			char str[60], message_str[100];
			sprintfC(str, "sample(%g,%g)", start, interval);
			DynLogEvents2(-1, 0, 1, str, 1, 0);
			sprintfC(message_str, "Sample event (%.60s) at time:", str);
			DymosimMessageDouble(message_str, Time);
		}
	}
  }
  return samp;
}
DYMOLA_STATIC double DYNTimeFloorEvent(int do_divide, double y, struct DYNInstanceData*did_, double time, Dymola_bool*AnyEvent, Dymola_bool*AnyDEvent, int sampleCounter, int pr, int DymolaOneIteration, const char*str) {
	double res = did_->NextSampleTime_vec[sampleCounter];
	int wasZero;
	if (!AnyEvent) return 0;
	wasZero = (0 == res);
	{
		const double currVal = do_divide ? (time / y) : (time*y);
		if (res > 0) res -= 1.0;
		if (wasZero || currVal<res || currVal >= res + 1) {
			if (AnyDEvent) *AnyDEvent = 1;
			if (pr) {
				DymosimMessageDouble("Time event at time:", time);
				DynLogEvents2(-1, 0, 1, str, 1, res);
			}
			*AnyEvent = 1;
			/* Update res */
			res = floor(currVal);
			did_->NextSampleTimeNew_vec[sampleCounter] = (res >= 0) ? res + 1 : res;
		}
		{
			/* Compute new time event point */
			double resNext = (y > 0) ? (res + 1) : (res);
			volatile double tNext = do_divide ? (resNext*y) : (resNext / y);
			struct BasicDDymosimStruct*basicD = getBasicDDymosimStruct();
			if (do_divide ? floor(tNext / y) == res : floor(tNext*y) == res) {
#if defined(_MSC_VER) ? (_MSC_VER>=1800) : (__STDC_VERSION__>199901L)
				tNext = nextafter(tNext, DBL_MAX)+1e-20*fabs((do_divide?(1/y):y));
#else
				tNext = tNext*(1 + DBL_EPSILON) + 1e-20*fabs((do_divide ? (1 / y) : y));
#endif
			}
			registerTimeEventNew(tNext, basicD);
		}
	}
	{
		return res;
	}
}
DYMOLA_STATIC Dymola_bool sampleFunctionM(struct DYNInstanceData*did_,double Time, double start, double interval, int counter,
                      Dymola_bool Init, Dymola_bool Event) {
  struct BasicDDymosimStruct*basicD=getBasicDDymosimStruct();
  Dymola_bool samp = false;
  if (interval<=0) DymosimErr("Sample did not have positive sample interval");
  if (Init|| (Event && NextSampleTime_[counter]==0)) {
	  double x;
	if (Init==2) return false; 
    basicD->mOrigTimeError=Dymola_max(basicD->mOrigTimeError,fabs(start)); /* Collect them */

	x=findCounter(Dymola_max(Time,start),start,interval);
	if (Init || x>NextSampleTime_[counter])
		NextSampleTime_[counter]=x;
    /* Samples at start,start+interval,...*/
    /* Replace Dymola_max(Time,start) by Time to sample at ...,start-interval,start,start+interval */
  };

  if (Event) {
    double eventTime=start+(NextSampleTime_[counter]-1)*interval;
    const double eventAccuracy=
#ifndef DynSimStruct
		5e-14
#else 
		1e-7
#endif
		*(fabs(Time)+basicD->mOrigTimeError);
    /* 5*eps to guard against different times */
      /*DymosimMessageDouble("Event at time: ",Time);*/
      /*DymosimMessageDouble("Event Time:",eventTime);*/
	NextSampleTimeNew_[counter]=NextSampleTime_[counter];
    while (eventTime<=Time+eventAccuracy) {
      NextSampleTimeNew_[counter]+=1;
      eventTime=start+(NextSampleTimeNew_[counter]-1)*interval;
      /*DymosimMessageDouble("Sampling at time: ", Time);*/
      /*DymosimMessageDouble("Next sampling",eventTime);*/
      samp = true;
    }

	NextSampleActNew_[counter]=1;
    registerTimeEventNew(eventTime, basicD); /* The next event for this sampler */

	samp = samp && (Iter == 1);
	{
		struct BasicIDymosimStruct* basicI = getBasicIDymosimStruct();
		if (samp && ((!FirstEvent && basicI->mPrintEvent&(1 << 1)) || (FirstEvent && basicI->mPrintEvent&(1 << 2)))) {
			char str[60], message_str[100];
			sprintfC(str, "sample(%g,%g)", start, interval);
			DynLogEvents2(-1, 0, 1, str, 1, 0);
			sprintfC(message_str, "Sample event (%.60s) at time:", str);
			DymosimMessageDouble(message_str,Time);
		}
	}
  }
  return samp;
}
DYMOLA_STATIC Dymola_bool sampleFunctionM3(struct DYNInstanceData*did_,double Time, double start, double interval, int phase, int maxVal, int counter,
                      Dymola_bool Init, Dymola_bool Event) {
	Dymola_bool samp = false;
	if (interval <= 0 || maxVal <= 0) DymosimErr("Sample did not have positive sample interval");
#if (defined(_OPENMP) && defined(DYMOLA_SUBPARA) && !defined(DISABLE_DYMOLA_OPENMP))
#pragma omp critical(TimeEvent) 
#endif
	{
	struct BasicDDymosimStruct*basicD = getBasicDDymosimStruct();
	double x, x2;

	if (Init != 2) {
	if (Init || (Event && NextSampleTime_[counter] == 0)) {
			basicD->mOrigTimeError = Dymola_max(basicD->mOrigTimeError, fabs(start)); /* Collect them */

			x = findCounter(Dymola_max(Time, start), start, interval);
			if (Init || x > NextSampleTime_[counter])
				NextSampleTime_[counter] = x;
			/* Samples at start,start+interval,...*/
			/* Replace Dymola_max(Time,start) by Time to sample at ...,start-interval,start,start+interval */
		}
	};

	if (Event) {
		double eventTime = start + (NextSampleTime_[counter] - 1)*interval;
		const double eventAccuracy =
#ifndef DynSimStruct
			5e-14
#else 
			1e-7
#endif
			*(fabs(Time) + basicD->mOrigTimeError);
		/* 5*eps to guard against different times */
		  /*DymosimMessageDouble("Event at time: ",Time);*/
		  /*DymosimMessageDouble("Event Time:",eventTime);*/
		NextSampleTimeNew_[counter] = NextSampleTime_[counter];
		while (eventTime <= Time + eventAccuracy) {
			NextSampleTimeNew_[counter] += 1;
			eventTime = start + (NextSampleTimeNew_[counter] - 1)*interval;
			/*DymosimMessageDouble("Sampling at time: ", Time);*/
			/*DymosimMessageDouble("Next sampling",eventTime);*/
			samp = true;
		}
		x = floor((0.1 + NextSampleTimeNew_[counter] - 2 - (maxVal - 1 - phase)) / maxVal); /* the passed point */
		x2 = x*maxVal + (maxVal - 1 - phase);
		samp = samp && (x2 == (NextSampleTimeNew_[counter] - 2)) && x >= 0; /* Handle negative phase */
		eventTime = start + (x2 + maxVal)*interval;
		NextSampleActNew_[counter] = 1;
		registerTimeEventNew(eventTime, basicD); /* The next event for this sampler */

		samp = samp && (Iter == 1);
		{
			struct BasicIDymosimStruct* basicI = getBasicIDymosimStruct();
			if (samp && ((!FirstEvent && basicI->mPrintEvent&(1 << 1)) || (FirstEvent && basicI->mPrintEvent&(1 << 2)))) {
				char str[60], message_str[100];
				sprintfC(str, "sample(%g,%g)", start, interval);
				DynLogEvents2(-1, 0, 1, str, 1, 0);
				sprintfC(message_str, "Sample event (%.60s) at time:", str);
				DymosimMessageDouble(message_str, Time);
			}
		}
	}
    }
	return samp;
}
#if defined(DYMOSIM) && defined(NI_)
LIBDS_API void InitI2(int, int, double*,int*);
static DYN_UNUSED_ATTR void InitI(struct DYNInstanceData* did_,int n,int d) {
	InitI2(n, d, QImd_, QIml_);
}
#else
static DYN_UNUSED_ATTR void InitI(struct DYNInstanceData* did_,int n,int d) {
	;
}
#endif
DYMOLA_STATIC void ClearNextSample(struct DYNInstanceData* did_) {
	int i;
	 for(i=0;i<NSamp_;++i) NextSampleActNew_[i]=0;
}

#if defined(DIRECT_FEED_THROUGH)
	  DYMOLA_STATIC int DirectFeedThrough_=1;
#else
	  DYMOLA_STATIC int DirectFeedThrough_=0;
#endif
#if defined(DYNForceEventInUpdate_)
	  DYMOLA_STATIC int DYNEventInUpdate_=DYNForceEventInUpdate_;
#else
	  DYMOLA_STATIC int DYNEventInUpdate_=1;
#endif


#if DymolaUseRDTSC_
#if defined(_MSC_VER) && (defined(_M_AMD64)||defined(_M_IX86))
#include <intrin.h>
#elif (defined(__GNUC__) || defined(__clang__)) && (defined(__amd64__) || defined(__i386__))
#include <x86intrin.h>
#elif defined (__LCC__)
extern long long _stdcall _rdtsc(void);
/* Lcc, default in Matlab, has it as an intrinsic function */
#else
#undef DymolaUseRDTSC_
#endif
#endif
#if DymolaUseRDTSC_
static double rtdrealFrequency=1.0e9; 
static double rtdinvFreq=1.0/1.0e9;
static const double MInt=4294967296.0;

static double DymolaPerformance(double*d,int i) {
	unsigned long long count=0;
	if (sizeof(count)!=sizeof(*d))
		return -1;
	{
		count=__rdtsc();
	}
	if (i==0) {
		if (d) *(unsigned long long*)(d)=count;
		return (double)(count);
	} else {
		unsigned long long a = *(unsigned long long*)(d);
		return (count-a) * rtdinvFreq;
	}
}
#if defined(_MSC_VER) && (defined(_M_AMD64)||defined(_M_IX86))
#define DYN_HAVE_CPUID 1
#elif (defined(__GNUC__)|| defined(__clang__)) && (defined(__amd64__) || defined(__i386__))
#include <cpuid.h>
#define DYN_HAVE_CPUID 2
#endif
struct DYM4Int {
	int vals[4];
};
static void InitializeFrequency(double d) {
	struct DYM4Int dummy;
	static int firstCall=1;
	if (!firstCall) return;
	firstCall=0;
	if (d==0) {
		unsigned int x;
		union {
			struct DYM4Int registers[3];
			char ch[48];
		} myValues;
		memset(myValues.ch,'0', sizeof(myValues));
		x = 0;
#if DYN_HAVE_CPUID==1
		__cpuid(dummy.vals, x);
		x = dummy.vals[0];
#elif DYN_HAVE_CPUID==2
		__cpuid(x, dummy.vals[0], dummy.vals[1], dummy.vals[2], dummy.vals[3]);
		x = dummy.vals[0];
#endif
		if (x >= 0x16) {
			x = 0x16;
#if DYN_HAVE_CPUID==1
			__cpuid(dummy.vals, x);
#elif DYN_HAVE_CPUID==2
			__cpuid(x, dummy.vals[0], dummy.vals[1], dummy.vals[2], dummy.vals[3]);
#endif
			d = dummy.vals[0] * 1e6;
			if (d != 0) {
				char str[400];
				sprintf(str, "Using processor base frequency %lg MHz\nNOTE: Only profiling information is impacted.\n", d / 1e6);
				DymosimMessage(str);
			}
		}
		if (d == 0) {
			x = 0x80000000UL;
#if DYN_HAVE_CPUID==1
			__cpuid(dummy.vals, x);
			x = dummy.vals[0];
#elif DYN_HAVE_CPUID==2
			__cpuid(x, dummy.vals[0], dummy.vals[1], dummy.vals[2], dummy.vals[3]);
			x = dummy.vals[0];
#endif
			if (x >= 0x80000004UL) {
				for (x = 0x80000002UL; x <= 0x80000004UL; ++x) {
#if DYN_HAVE_CPUID
#if DYN_HAVE_CPUID==1
					__cpuid(dummy.vals, x);
#else
					__cpuid(x, dummy.vals[0], dummy.vals[1], dummy.vals[2], dummy.vals[3]);
#endif
					myValues.registers[x - 0x80000002UL] = dummy;
#endif
				}
				{
					double dMult = 1e6;
					char* lastM = strrchr(myValues.ch, 'M');
					if (lastM != 0 && lastM[1] == 'H' && lastM[2] == 'z') {
					} else {
						lastM = strrchr(myValues.ch, 'G');
						if (lastM != 0 && lastM[1] == 'H' && lastM[2] == 'z') {
							dMult = 1e9;
						} else lastM = 0;
					}
					if (lastM != 0) {
						for (; lastM > myValues.ch && lastM[-1] == ' '; lastM--);
						for (; lastM > myValues.ch && (lastM[-1] >= '0' && lastM[-1] <= '9' || lastM[-1] == '.'); lastM--);
						if (sscanf(lastM, "%lg", &d) != 1) {
							d = 0;
						} else d *= dMult;
					}
				}

			}
			if (d == 0) {
				char str[400];
				sprintf(str, "Could not determine speed of processor. Assuming 1GHz\nCPU reported: %s\nNOTE: Only profiling information is impacted.\n", myValues.ch);
				DymosimMessage(str);
				d = 1e9;
			} else {
				char str[400];
				sprintf(str, "Determined processor speed to %lg MHz\nCPU reported: %s\nNOTE: Only profiling information is impacted.\n", d / 1e6, myValues.ch);
				DymosimMessage(str);
			}
		}
	}
	rtdrealFrequency=d;
	rtdinvFreq=1.0/rtdrealFrequency;
	{
		extern double (*DymolaTimerCounterCallback)(double*,int);
		DymolaTimerCounterCallback=&DymolaPerformance;
	}
}
#if defined(DymolaUseRDTSCFrequency_)
#define SetupProcessorCounter() InitializeFrequency(DymolaUseRDTSCFrequency_)
#else 
#define SetupProcessorCounter() InitializeFrequency(0.0)
#endif

#else
#define SetupProcessorCounter()
#endif

DYMOLA_STATIC void GetDimensions(int *nx_, int *nx2_, int *nu_, int *ny_, int *nw_, int *np_, 
  int *nrel_, int *ncons_, int *dae_)
{
      *nx_ = NX_;
      *nx2_ = NX2_; 
      *nu_ = NU_;
      *ny_ = NY_;
      *nw_ = NW_;
      *np_ = NP_;
      *nrel_ = NRel_;
      *ncons_ = NCons_;
      *dae_ = DAEsolver_;
#if  defined(DynSimStruct)
	  SetupProcessorCounter();
#endif
}

DYMOLA_STATIC void GetDimensions2(int *nx_, int *nx2_, int *nu_, int *ny_, int *nw_, int *np_, int* nsp_,
  int*nrel2_,int *nrel_, int *ncons_, int *dae_)
{
      *nx_ = NX_;
      *nx2_ = NX2_; 
      *nu_ = NU_;
      *ny_ = NY_;
      *nw_ = NW_;
#ifdef NPS_
	  *nsp_ = NPS_;
#else
	  *nsp_ = 0;
#endif
      *np_ = NP_;
      *nrel_ = NRel_;
#ifdef NRelF_
	  *nrel2_ = NRelF_;
#else
	  *nrel2_ = NRel_;
#endif
      *ncons_ = NCons_;
      *dae_ = DAEsolver_;
#if  defined(DynSimStruct)
	  SetupProcessorCounter();
#endif
}

DYMOLA_STATIC void GetDimensions5(int *nx_, int *nx2_, int *nu_, int *ny_, int *nw_, int *np_, int* nsp_,
  int*nrel2_,int *nrel_, int *ncons_, int *dae_, int *nd_, int* nxp_, int* nwp_, int* nwc_,int *nhelp2_, int *naux_){
	  *nx_ = NX_;
      *nx2_ = NX2_; 
      *nu_ = NU_;
      *ny_ = NY_;
      *nw_ = NW_;
#ifdef NPS_
	  *nsp_ = NPS_;
#else
	  *nsp_ = 0;
#endif
      *np_ = NP_;
      *nrel_ = NRel_;
#ifdef NRelF_
	  *nrel2_ = NRelF_;
#else
	  *nrel2_ = NRel_;
#endif
      *ncons_ = NCons_;
      *dae_ = DAEsolver_;
	  *nd_ = ND_;
	  *nxp_ = NXP_;
	  *nwp_ = NWP_;
	  *nwc_ = NWC_;
	  *nhelp2_ = NGlobalHelp_;

#if  defined(DynSimStruct)
	  SetupProcessorCounter();
#endif
	  * naux_ = MAXAux_;
}
DYMOLA_STATIC void GetDimensions4(int* nx_, int* nx2_, int* nu_, int* ny_, int* nw_, int* np_, int* nsp_,
	int* nrel2_, int* nrel_, int* ncons_, int* dae_, int* nd_, int* nxp_, int* nwp_, int* nwc_, int* nhelp2_) {
	/* To remove */
	int naux;
	GetDimensions5(nx_, nx2_, nu_, ny_, nw_, np_, nsp_, nrel2_, nrel_, ncons_, dae_, nd_, nxp_, nwp_, nwc_, nhelp2_, &naux);
}
DYMOLA_STATIC void GetNbrConstAux(int* nwp_) {
	*nwp_ = NWP_;
}
static int nx_=NX_;
static int nx2_=NX2_;
static int nu_=NU_;
static int ny_=NY_;
static int nw_=NW_;
static int np_=NP_;
#ifdef NPS_
static int nsp_=NPS_;
#else
static int nsp_=0;
#endif
#ifdef NRelF_
static int nrel2_=NRelF_;
#else
static int nrel2_=NRel_;
#endif
static int nrel_=NRel_;
static int ncons_=NCons_;
static int dae_=DAEsolver_;

DYMOLA_STATIC void InitializeDymosimStruct(struct BasicDDymosimStruct*basicD,struct BasicIDymosimStruct*basicI) {
#if INLINE_INTEGRATION
	  basicI->mInlineIntegration=INLINE_INTEGRATION;
#else
	  basicI->mInlineIntegration=0;
#endif
	  basicI->mUsingCVodeGodess = 0;
	  basicI->mBlockUnblockSmoothCrossings = 0;
#ifdef PrintEvent_
	  basicI->mPrintEvent = PrintEvent_;
#endif
#if defined(DymolaGeneratedFixedStepSize_)
	 basicD->mDymolaFixedStepSize=DymolaGeneratedFixedStepSize_;
#else
	 basicD->mDymolaFixedStepSize=0.0;
#endif
	 basicD->mCurrentStepSizeRatio2 = 1.0;
	 basicD->mOrigTimeError=0;
#if defined(FMUStoreResultInterval_)
	 basicD->mStoreResultInterval = FMUStoreResultInterval_;
#else
	 basicD->mStoreResultInterval = 0;
#endif
}

#if defined(RT) || defined(NRT)
#else

DYMOLA_STATIC int dsblockb_(const int *iopt_, int info_[], int sig_[], int dim_[], 
        double *t0_, double x0_[], double xd0_[], 
        double dp_[], int ip_[], Dymola_bool lp_[], 
		double duser_[], int iuser_[], void*cuser_[], 
        int *QiErr)
  {
    int /* c1_, c2_, c3_, i_, */ nx2_;
    /* double d1_; */

	*(GlobalErrorPointer())=0;
    if (DAEsolver_)
      nx2_ = NX2_;
    else
      nx2_ = 0;

    if (*iopt_ == 1) {	

    } else if (*iopt_ == 2) {
	
	  SetupProcessorCounter();

      if (NCons_ > 0)
        info_[0] = 2;
      else if (DAEsolver_)
        info_[0] = 1;
#if defined(HaveDummyDerivative_)
	  info_[1] = 1;
#endif

#if defined(AnalyticJacobian_)
	  info_[2] = AnalyticJacobianElements_;
#elif AnalyticDirectionalJac_==2
	  info_[2] = -2;
#elif defined(AnalyticDirectionalJac_)
	  info_[2] = -1;
#endif
      /* if (NRel_ > 0 && NX_ + nx2_ == 0) */
      if (NX_ + nx2_ == 0)
        sig_[0] = 1;       /* To enable handling of "state events" without states. */
      else 
        sig_[0] = NX_ + nx2_;
      sig_[1] = NU_;
      sig_[2] = NY_;
      if (NRel_ > 0 && RootFinder_)
        sig_[3] = 1;
      sig_[4] = NW_;
      sig_[5] = NP_;
      sig_[6] = NA_;   /* Number of alias signal matrices or scalars */
      sig_[7] = NA_;   /* Total number of alias elements */
  
      /* if (NRel_ > 0 && NX_ + nx2_ == 0) */
      if (NX_ + nx2_ == 0)
        dim_[0] = 1;
      else
        dim_[0]  = NX_ + nx2_;
      dim_[1]  = NU_;
      dim_[2]  = NY_;
      dim_[3]  = 2 * NRel_;
      dim_[4]  = NW_;
      dim_[5]  = NP_;
	  dim_[6]  = NRelF_;
	  dim_[7]  = SizeEq_;
      dim_[9]  = NHash1_;
      dim_[10] = NHash2_;
      dim_[11] = NX_;
      dim_[12] = nx2_;
	  dim_[13] = (int)(NGlobalHelp_);
	  dim_[14] = NHash3_;
#ifdef NPS_
      dim_[15] = NPS_;
#endif
	  dim_[19] = NWP_;
	  dim_[20] = NWC_;

      dim_[8] = dim_[0] + dim_[2] + dim_[3]*sig_[3] + dim_[4] + sizeof(struct BasicDDymosimStruct)/sizeof(doublereal);
      
    } else if (*iopt_ == 3) { 
      iuser_[0] = NX_ + nx2_ + NCons_;
      iuser_[1] = NY_;
      iuser_[2] = NW_;
      iuser_[3] = 2 * NRel_;
      InitializeDymosimStruct((struct BasicDDymosimStruct*)(duser_+
		   iuser_[0]+iuser_[1]+iuser_[2]+iuser_[3]),(struct BasicIDymosimStruct*)(iuser_+4));

	  


      /* if (NRel_ > 0 && NX_ + nx2_ == 0) */
      
			

      declareNew_(x0_, dp_, 0, cuser_, QiErr, 0, (struct DeclarePhase*)0);
    } 
	else if (*iopt_ == 4) {
		declareNew_(x0_,dp_,0,cuser_,QiErr, 1, (struct DeclarePhase*)0);
	} else if (*iopt_ == 5) {
		InitializeDymosimStruct((struct BasicDDymosimStruct*)(duser_),(struct BasicIDymosimStruct*)(iuser_));
	}



leave: DYN_UNUSED_LABEL_ATTR
  if (*(GlobalErrorPointer()) != 0)
    *QiErr = *(GlobalErrorPointer());
  return 0;
}
#endif

void DYNInterpolateInputs(double* U_, double t, struct DYNInstanceData* did_) {
	if (NU_ == 0) return;
#if defined(DYN_HAVE_MYCHANGE_INPUTS)
	{
		/* For standalone */
		MyChangeInputs(NU_, U_, t);
    }
#endif
#if defined(DYMOSIM) && !defined(GODESS) && !defined(LIBDS_DLL)
	{
		/* Remove DLL-part later */
		LIBDS_API int aarsa4_(integer * ideval, doublereal * ti, doublereal ui[]);
		integer dummy;
		aarsa4_(&dummy, &t, U_);
	}
#endif
}

DYMOLA_STATIC int QJacobianDefined_ =
#if AnalyticDirectionalJac_==2
24|
#elif AnalyticDirectionalJac_==1
8|
#endif
#if defined(AnalyticAdjoint_)
4|
#endif
#if defined(AnalyticJacobian_) && defined(AnalyticJacobianBCD_)
3;
#elif defined(AnalyticJacobian_)
1;
#else
0;
#endif

#if !defined(QJacobianCGDef_)
DYMOLA_STATIC int QJacobianCG_[3]={0,0,0};
DYMOLA_STATIC struct QJacobianTag_ QJacobianGC2_[1]={{0,0}};
DYMOLA_STATIC double QJacobianCD_[NX_+1]={0};
#endif

#if !defined(DYN_COMPILE_WITH_SPARSE) && !defined(FMU_SOURCE_CODE_EXPORT_SPARSE)
DYMOLA_STATIC struct SuperLUInterface *superlu_enabled(int nx, int analytic_jacobian, int nnz, const int nxOrig, const int cgOffset, const int gcOffset) { return 0; }
struct SparseFunctions;
DYMOLA_STATIC void DSSetSparseFunctions(struct SparseFunctions* functions) { ; }
#endif

/* vector to hold FMI value references for possible continuous time states*/
#if !defined(FMIStateValueReferencesDef_)
DYMOLA_STATIC unsigned int FMIStateValueReferences_[1]={~0};
#endif

DYMOLA_STATIC void GetNonlinearSolverStatistics(int*const qnlmax, const int**const qnlfunc, 
	const int**const qnljac, const int**const qnlcalls, int*const nrtimers)
{
	if (qnlmax) *qnlmax = QNLmax;
	if (qnlfunc) *qnlfunc = QNLfunc;
	if (qnljac) *qnljac = QNLjac;
	if (qnlcalls) *qnlcalls = QNLcalls;
#ifdef NrDymolaTimers_
	if (nrtimers) *nrtimers = NrDymolaTimers_;
#else
	if (nrtimers) *nrtimers = 0;
#endif
}

DYMOLA_STATIC DYN_UNUSED_ATTR void RealNonTempSetSize(RealArray* x, SizeType ndims, SizeType* dims);
DYMOLA_STATIC DYN_UNUSED_ATTR void IntegerNonTempSetSize(IntegerArray* x, SizeType ndims, SizeType* dims);
DYMOLA_STATIC DYN_UNUSED_ATTR void StringNonTempSetSize(StringArray* x, SizeType ndims, SizeType* dims);
DYMOLA_STATIC DYN_UNUSED_ATTR void RealNonTempCopyData(RealArray* x, const Real* data, SizeType nbrData);
DYMOLA_STATIC DYN_UNUSED_ATTR void IntegerNonTempCopyData(IntegerArray* x, const Integer* data, SizeType nbrData);
DYMOLA_STATIC DYN_UNUSED_ATTR void StringNonTempCopyData(StringArray* x, const String* data, SizeType nbrData);
DYMOLA_STATIC  void SetRealDymArraySize(struct DYNInstanceData* did_, int index, int ndims, int* dims) {
#ifdef  NAR_
	DYNInstanceData* did = did_ ? did_ : &tempData;
	RealNonTempSetSize(&did->UR_[index], ndims, dims);
#endif //  NAR_
}

DYMOLA_STATIC void SetRealDymArrayValues(struct DYNInstanceData* did_, int index, const double* data, int nbrData) {
#ifdef  NAR_
	DYNInstanceData* did = did_ ? did_ : &tempData;
	RealNonTempCopyData(&did->UR_[index], data, nbrData);
#endif //  NAR_
}

DYMOLA_STATIC void GetRealDymArrayValues(struct DYNInstanceData* did_, int index, double* data, int nbrData) {
#ifdef  NAR_
	DYNInstanceData* did = did_ ? did_ : &tempData;
	memcpy(data, did->UR_[index].data, nbrData*sizeof(double));
#endif //  NAR_
}

DYMOLA_STATIC  void SetIntegerDymArraySize(struct DYNInstanceData* did_, int index, int ndims, int* dims) {
#ifdef  NAI_
	DYNInstanceData* did = did_ ? did_ : &tempData;
	IntegerNonTempSetSize(&did->UI_[index], ndims, dims);
#endif //  NAI_
}

DYMOLA_STATIC void SetIntegerDymArrayValues(struct DYNInstanceData* did_, int index, const int* data, int nbrData) {
#ifdef  NAI_
	DYNInstanceData* did = did_ ? did_ : &tempData;
	IntegerNonTempCopyData(&did->UI_[index], data, nbrData);
#endif //  NAI_
}

DYMOLA_STATIC void GetIntegerDymArrayValues(struct DYNInstanceData* did_, int index, int* data, int nbrData) {
#ifdef  NAI_
	DYNInstanceData* did = did_ ? did_ : &tempData;
	memcpy(data, did->UI_[index].data, nbrData*sizeof(int));
#endif //  NAI_
}

DYMOLA_STATIC  void SetStringDymArraySize(struct DYNInstanceData* did_, int index, int ndims, int* dims) {
#ifdef  NAS_
	DYNInstanceData* did = did_ ? did_ : &tempData;
	DYNInstanceData(&did->US_[index], ndims, dims);
#endif //  NAS_
}

DYMOLA_STATIC void SetStringDymArrayValues(struct DYNInstanceData* did_, int index, const char** data, int nbrData) {
#ifdef  NAS_
	DYNInstanceData* did = did_ ? did_ : &tempData;
	StringNonTempCopyData(&did->US_[index], data, nbrData);
#endif //  NAS_
}


DYMOLA_STATIC void getDelayStruct(struct DYNInstanceData* did_, delayStruct** del, size_t* nbrDel) {
	if(nbrDel)
		*nbrDel = SizeDelay_ ? SizeDelay_ : 1;
	if (del) {
		if (did_) {
			*del = did_->del_vec;
		}else {
			*del = tempData.del_vec;
		}
	}
}

DYMOLA_STATIC void getFMUStorage(const int ** p) {
	if (p) {
#if defined(FMUStoreResult_)
		*p = fmuStorage;
#else
		*p = 0;
#endif
	}
}


DYMOLA_STATIC int GetAdditionalFlags(int flag_num)
{
	switch (flag_num) {
	case 1:
#if defined(DYNSparseJacobian_) && (defined(DYN_COMPILE_WITH_SPARSE) || defined(FMU_SOURCE_CODE_EXPORT_SPARSE))
		return DYNSparseJacobian_;
#else
		return 0;
#endif
		break;
	case 2:
#ifdef DYNDAEsolverErrorControlAlgebraicVariables_
		return DYNDAEsolverErrorControlAlgebraicVariables_;
#else
		return 1;
#endif
		break;
	case 3:
#ifdef DYNResultSnapshotTimeInFileName_
		return DYNResultSnapshotTimeInFileName_;
#else
		return 1;
#endif
		break;
	case 4:
#ifdef DYNHasTerminal_
		return DYNHasTerminal_;
#else
		return 0;
#endif
		break;
	case 5:
#ifdef DYNNoSimulationTiming_
		return DYNNoSimulationTiming_;
#else
		return 0;
#endif
		break;
	case 6:
	{
		int ret = 15;
#ifdef DYNLogTimeEvents_
		ret -= 3 * (!DYNLogTimeEvents_);
#endif
#ifdef DYNLogStateEvents_
		ret -= 4 * (!DYNLogStateEvents_);
#endif
#ifdef DYNLogStepEvents_
		ret -= 8 * (!DYNLogStepEvents_);
#endif
		return ret;
		break;
	}
	case 8:
#ifdef DYNDymosimSleep_
		return DYNDymosimSleep_;
#else
		return 0;
#endif
		break;
	case 9:
#ifdef DYNSteadyStateTermination_
		return DYNSteadyStateTermination_;
#else
		return 0;
#endif
		break;
	case 10:
#ifdef DYNFailIfSteadyStateNotReached_
		return DYNFailIfSteadyStateNotReached_;
#else
		return 0;
#endif
		break;
	case 11:
#ifdef DYNSteadyState_
		return DYNSteadyState_;
#else
		return 0;
#endif
		break;
	case 12:
#ifdef DYNGodessExtendedEventChecking_
		return DYNGodessExtendedEventChecking_;
#else
		return 1;
#endif
		break;	
	case 13:
#ifdef DYNInlineMethod_
		return DYNInlineMethod_;
#else
		return 0;
#endif
		break;	
	case 14:
#ifdef DYNInlineOrder_
		return DYNInlineOrder_;
#else
		return 2;
#endif
		break;
	case 15:
#ifdef DYNSteadyStateTerminationStrict_
		return DYNSteadyStateTerminationStrict_;
#else
		return 0;
#endif
		break;	
	case 16:
#ifdef DYNSmallDefaultPerturbationForIntegratorJacobian_
		return DYNSmallDefaultPerturbationForIntegratorJacobian_;
#else
		return 0;
#endif
		break;
	case 17:
#if !defined(DYNGenerateTimers_)
		return 0;
#endif
#ifdef DYNWriteTimerResultsToFile_
		return DYNWriteTimerResultsToFile_;
#else
		return 1;
#endif
		break;
	case 18:
#ifdef DYNPerformAdditionalEventIterationWhenEventAtTerminal_
		return DYNPerformAdditionalEventIterationWhenEventAtTerminal_;
#else
		return 1;
#endif
		break;
	case 20:
#ifdef DYNCvodePreciseOutputTimeCalculation_
		return DYNCvodePreciseOutputTimeCalculation_;
#else
		return 1;
#endif
		break;
	case 21:
#ifdef DYNSteadyStateIgnoreLowOrderStates_
		return DYNSteadyStateIgnoreLowOrderStates_;
#else
		return 0;
#endif
		break;
	case 22:
#if defined(HaveDummyDerivative_) && defined(DYNSteadyStateDetectionDummyDerivatives_)
		return !DYNSteadyStateDetectionDummyDerivatives_;
#else
		return 0;
#endif
		break;
	case 23:
#ifdef NSparse_
		return NSparse_;
#else
		return 0;
#endif
		break;
	case 24:
#ifdef DYNForceStopAtTimeEvents_
		return DYNForceStopAtTimeEvents_;
#else
		return 0;
#endif
		break;
	case 25:
#ifdef DYNFMI2CVodeSetStopTimeWhenUsingDerivatives_
		return DYNFMI2CVodeSetStopTimeWhenUsingDerivatives_;
#else
		return 0;
#endif
		break;
	case 26:
#ifdef DYNLinearSolverConsistencyTestScaleResidual_
		return DYNLinearSolverConsistencyTestScaleResidual_;
#else
		return 1;
#endif
		break;
	case 27:
#ifdef DYNIda_
		return DYNIda_;
#else
		return 0;
#endif
		break;
	case 28:
#ifdef DYNSuperLU_MTMemoryFactor_
		return DYNSuperLU_MTMemoryFactor_;
#else
		return 0;
#endif
		break;
	case 29:
#ifdef DYNUseInputSmoother_
		return DYNUseInputSmoother_;
#else
		return 0;
#endif
		break;
	case 30:
#ifdef DYNUsePredictorCompensation_
		return DYNUsePredictorCompensation_;
#else
		return 0;
#endif
		break;
	case 31:
#ifdef AnalyticJacobianElements_
		return AnalyticJacobianElements_;
#else
		return 0;
#endif
		break;
	case 32:
#ifdef DYNBifurcationWarning_
		return DYNBifurcationWarning_;
#else
		return 1;
#endif
		break;
	case 33:
#ifdef DYNCentralDifferenceJacobianDassl_
		return DYNCentralDifferenceJacobianDassl_;
#else
		return 0;
#endif
		break;
	case 34:
#ifdef DYNPredictorCompensationCache_
		return DYNPredictorCompensationCache_;
#else
		return 0;
#endif
		break;
	case 35:
#ifdef DYNZeroOrderHoldWithoutReinit_
		return DYNZeroOrderHoldWithoutReinit_;
#else
		return 0;
#endif
		break;
	case 37:
#ifdef DYNGenerateVariableDependencies_
		return DYNGenerateVariableDependencies_;
#else
		return 0;
#endif
		break;
	case 38:
#ifdef DYNGenerateBlockTimers_
		return DYNGenerateBlockTimers_;
#else
		return 0;
#endif
		break;
	case 39:
#ifdef DYNGenerateFunctionTimers_
		return DYNGenerateFunctionTimers_;
#else
		return 0;
#endif
		break;
	case 40:
#ifdef DYNExposeDAEResiduals_
		return DYNExposeDAEResiduals_;
#else
		return 0;
#endif
		break;
	default:
		return 0;
	}
}

DYMOLA_STATIC void GetArrayPointer(const unsigned int ** arrayVrs,const size_t ** arraySz) {
#if defined(FMI_ARRAY_DETAILS)
	*arrayVrs = arrayInternalValueReference;
	*arraySz = arraySizes;
#else
	*arrayVrs = NULL;
	*arraySz = NULL;
#endif
}

DYMOLA_STATIC void GetNbrDynArray(size_t* nbrReal, size_t* nbrInt, size_t* nbrString) {
#ifdef NAR_
	* nbrReal = NAR_;
#else
	* nbrReal = 0;
#endif
#ifdef NAI_
	* nbrInt = NAI_;
#else
	* nbrInt = 0;
#endif
#ifdef NAS_
	* nbrString = NAS_;
#else
	* nbrString = 0;
#endif
}

DYMOLA_STATIC double GetAdditionalReals(int flag_num)
{
	switch (flag_num) {
	case 1:
#ifdef DYNResultSnapshotInterval_
		return DYNResultSnapshotInterval_;
#else
		return 0.0;
#endif
		break;
	case 2:
#ifdef DYNSteadyStateTerminationTolerance_
		return DYNSteadyStateTerminationTolerance_;
#else
		return 0.0;
#endif
		break;	
	case 3:
#ifdef DYNInlineFixedStep_
		return DYNInlineFixedStep_;
#else
		return 0.001;
#endif
		break;
	case 4:
#ifdef DYNMaxRunTimePerSimulation_
		return DYNMaxRunTimePerSimulation_;
#else
		return 0.0;
#endif
		break;
	case 5:
#ifdef DYNSteadyStateTerminationStartTime_
		return DYNSteadyStateTerminationStartTime_;
#else
		return -1.0e308;
#endif
		break;
	case 6:
#ifdef DYNNominalRescalingForIntegratorJacobian_
		return DYNNominalRescalingForIntegratorJacobian_;
#else
		return 1.0e-3;
#endif
		break;
	case 7:
#ifdef DYNCutOffForIntegratorJacobianLow_
		return DYNCutOffForIntegratorJacobianLow_;
#else
		return 8.0e-17;
#endif
		break;
	case 8:
#ifdef DYNCutOffForIntegratorJacobianHigh_
		return DYNCutOffForIntegratorJacobianHigh_;
#else
		return 1.0e-10;
#endif
		break;
	case 9:
#ifdef DYNFixedPerturbationForIntegratorJacobian_
		return DYNFixedPerturbationForIntegratorJacobian_;
#else
		return -1.5e-8;
#endif
		break;	
	case 10:
#ifdef DYNSteadyStateSecondDerivativeToleranceScaling_
		return DYNSteadyStateSecondDerivativeToleranceScaling_;
#else
		return 0.0;
#endif
		break;
	default:
		return 0.0;
	}
}

DYMOLA_STATIC int fmiUser_Instantiate() {
#ifdef FMI_USER_INSTANTIATE
	return FMI_USER_INSTANTIATE();
#else
	return 0;
#endif
}
DYMOLA_STATIC int fmiUser_Initialize() {
#ifdef FMI_USER_INITIALIZE
	return FMI_USER_INITIALIZE();
#else
	return 0;
#endif
}
DYMOLA_STATIC int fmiUser_Terminate() {
#ifdef FMI_USER_TERMINATE
	return FMI_USER_TERMINATE();
#else
	return 0;
#endif
}
DYMOLA_STATIC void fmiUser_Free() {
#ifdef FMI_USER_FREE
	FMI_USER_FREE();
#endif
}
DYMOLA_STATIC int fmuUser_Reset() {
#ifdef FMI_USER_RESET
	return FMI_USER_RESET();
#else
	return 0;
#endif
}

/* End dsblock5.c */

