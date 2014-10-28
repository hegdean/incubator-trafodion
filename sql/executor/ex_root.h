/**********************************************************************
// @@@ START COPYRIGHT @@@
//
// (C) Copyright 1994-2014 Hewlett-Packard Development Company, L.P.
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//
// @@@ END COPYRIGHT @@@
**********************************************************************/
#ifndef EX_ROOT_H
#define EX_ROOT_H
/* -*-C++-*-
 *****************************************************************************
 *
 * File:         ex_root.h
 * Description:  The root TCB is the root of a TCB tree in the master
 *               executor. It interacts with the CLI and deals with
 *               things like transfer of data from/to user host variables.
 *               
 * Created:      4/15/95
 * Language:     C++
 *
 *
 *
 *
 *****************************************************************************
 */

// -----------------------------------------------------------------------
#include "cli_stdh.h"

#include "ex_exe_stmt_globals.h"
#include "exp_expr.h"
#include "ComSqlexedbg.h"
#include "SqlTableOpenInfo.h"
#include "Ipc.h"
#include "rts_msg.h"

class InputExpr;
class OutputExpr;
class Descriptor;
class ExRtFragTable;
class TransMode;
class ComDiagsArea;

//
// Task Definition Block
//
#include "ComTdbRoot.h"

// -----------------------------------------------------------------------
// Classes defined in this file
// -----------------------------------------------------------------------
class ex_root_tdb;
class ex_root_tcb;
// -----------------------------------------------------------------------
// Classes referenced in this file
// -----------------------------------------------------------------------
class ex_tcb;
class ExMasterStats;

// -----------------------------------------------------------------------
// ex_root_tdb
// -----------------------------------------------------------------------
#pragma nowarn(1319)   // warning elimination 
class ex_root_tdb : public ComTdbRoot
{
public:

  // ---------------------------------------------------------------------
  // Constructor is only called to instantiate an object used for
  // retrieval of the virtual table function pointer of the class while
  // unpacking. An empty constructor is enough.
  // ---------------------------------------------------------------------
  NA_EIDPROC ex_root_tdb()
  {}

  NA_EIDPROC virtual ~ex_root_tdb()
  {}

  // ---------------------------------------------------------------------
  // Build a TCB for this TDB. Redefined in the Executor project.
  // ---------------------------------------------------------------------
  NA_EIDPROC virtual ex_tcb *build(CliGlobals *cliGlobals, 
                                   ex_globals *globals);

private:
  // ---------------------------------------------------------------------
  // !!!!!!! IMPORTANT -- NO DATA MEMBERS ALLOWED IN EXECUTOR TDB !!!!!!!!
  // *********************************************************************
  // The Executor TDB's are only used for the sole purpose of providing a
  // way to supplement the Compiler TDB's (in comexe) with methods whose
  // implementation depends on Executor objects. This is done so as to
  // decouple the Compiler from linking in Executor objects unnecessarily.
  //
  // When a Compiler generated TDB arrives at the Executor, the same data
  // image is "cast" as an Executor TDB after unpacking. Therefore, it is
  // a requirement that a Compiler TDB has the same object layout as its
  // corresponding Executor TDB. As a result of this, all Executor TDB's
  // must have absolutely NO data members, but only member functions. So,
  // if you reach here with an intention to add data members to a TDB, ask
  // yourself two questions:
  //
  // 1. Are those data members Compiler-generated?
  //    If yes, put them in the ComTdbRoot instead.
  //    If no, they should probably belong to someplace else (like TCB).
  // 
  // 2. Are the classes those data members belong defined in the executor
  //    project?
  //    If your answer to both questions is yes, you might need to move
  //    the classes to the comexe project.
  // ---------------------------------------------------------------------
};
#pragma warn(1319)  // warning elimination 




/////////////////////////////////////
// Task control block
/////////////////////////////////////
#pragma nowarn(1319)   // warning elimination 
class ex_root_tcb : public ex_tcb
{

  friend class ex_root_tdb;

public:
  ex_root_tcb(const ex_root_tdb & root_tdb,
	      const ex_tcb & child_tdb,
	      ExExeStmtGlobals *glob);
  
  ~ex_root_tcb();
  void        freeResources();  // free resources
  void        registerSubtasks();
  
  short       work();  // when scheduled to do work
  
  virtual Int32 fixup();

  // if afterRecomp is TRUE, indicates that re-execute is being done
  // after a lost open or automatic recompilation of this query.
  Int32 execute(CliGlobals *cliGlobals, ExExeStmtGlobals * glob,
              Descriptor * input_desc, ComDiagsArea* &diagsArea,
	      NABoolean reExecute = FALSE);

  // closeCursorOnError: if an error is returned for a cursor statement
  //     and this param is returned as TRUE, then the cursor is closed.
  //     Otherwise the cursor remains open.
  //     This param is set to FALSE on return if an error occurs during
  //     output value processing, like an overflow error into a hostvar,
  //     or a missing null indicator. The cursor remains open and the next
  //     fetch would get the next row. This is ANSI compliant behavior.
  //     In all other cases, this param is returned as TRUE.
  Int32 fetch(CliGlobals *cliGlobals, ExExeStmtGlobals * glob,
            Descriptor * output_desc, ComDiagsArea* &diagsArea,
            Lng32 timeLimit, NABoolean newOperation,
	    NABoolean &closeCursorOnError);

  Int32 fetchMultiple(CliGlobals *cliGlobals, ExExeStmtGlobals * glob,
		    Descriptor * output_desc, ComDiagsArea* &diagsArea,
		    Lng32 timeLimit, NABoolean newOperation,
		    NABoolean &closeCursorOnError, NABoolean &eodSeen);

  Int32 oltExecute(ExExeStmtGlobals * glob, Descriptor * input_desc,
		  Descriptor * output_desc,
		 ComDiagsArea* &diagsArea);
 
  Int32 requestCancel();

  // Called by the main thread for internal cancel.
  Int32 cancel(ExExeStmtGlobals * glob, ComDiagsArea* &diagsArea, 
             NABoolean getQueueDiags = FALSE);
  Int32 deallocAndDelete(ExExeStmtGlobals * glob,ExRtFragTable *fragTable);
  Int32 closeTables(ExExeStmtGlobals * glob,
		  ExRtFragTable * fragTable);
  Int32 reOpenTables(ExExeStmtGlobals * glob,
		   ExRtFragTable * fragTable);

  char * getPkeyRow();

  // This function passes the diagnostics area from the entry
  // to the cli.
  inline ComDiagsArea *moveDiagsAreaFromEntry(ex_queue_entry *entry)
    {
  	ComDiagsArea *da = entry->getDiagsArea();
	  if (da!=NULL)
	    da->incrRefCount();
	  return da;
    }

  inline ex_root_tdb & root_tdb() const
    {
      return (ex_root_tdb &) tdb;
    };

// warning elimination (removed "inline")
  ex_queue_pair getParentQueue() const 
    { 
      ex_queue_pair temp;
      temp.down = NULL;
      temp.up   = NULL;
      return temp; 
    }

  inline InputOutputExpr * inputExpr() const { return root_tdb().inputExpr_; }
  inline InputOutputExpr * outputExpr() const {return root_tdb().outputExpr_;}
  inline ex_expr * pkeyExpr() const { return root_tdb().pkeyExpr_; }
  inline ex_expr * predExpr() const { return root_tdb().predExpr_; }

  virtual NABoolean needStatsEntry();

  virtual ExOperStats * doAllocateStatsEntry(CollHeap *heap, ComTdb *tdb);

  // this method receives the row of primary key values for an update
  // where current of query. The row is returned from the select cursor
  // thru the getPkeyRow() method.
  void inputPkeyRow(char * pkey_row);

  virtual Int32 numChildren() const { return 1; }   
  virtual const ex_tcb* getChild(Int32 pos) const;

  NABoolean externalEventCompleted(void);

  // ****  information for GUI  *** -------------
  inline Int32 displayExecution() { return root_tdb().displayExecution(); }
  //---------------------------------------------
   
  inline short getTableCount() {return root_tdb().tableCount_; }
  inline SqlTableOpenInfoPtr *getStoiList() {return root_tdb().stoiList_; }

  void getInputData(char* &inputData, ULng32 &inputDatalen);
  void setInputData(char* inputData);

  // ++ triggers
  inline char * getTriggerStatusVector() { return triggerStatusVector_; }
  inline void setTriggerStatusVector(char* tsv) { triggerStatusVector_ = tsv; }
  // -- triggers
 
  inline NABoolean fatalErrorOccurred() { return fatalError_; }
  void setFatalError() { fatalError_ = TRUE; }

  // Let the cancel broker know this query is finished.
  void deregisterQuery();

  // Enforce query CPU limit.
  NA_EIDPROC virtual void cpuLimitExceeded();

private:

  const ex_tcb * tcbChild_;

  ex_queue_pair	qchild;

  // Atp and buffers to build insert data
  atp_struct * workAtp_;

  // Atp to compute the primary key row 
  atp_struct * pkeyAtp_;

  Descriptor * input_desc;
  Descriptor * output_desc;

  char * rwrsBuffer_;

  // For the query limits and async cancel to schedule work().
  ExSubtask * asyncCancelSubtask_;
  // TRUE iff work() starts cancel but not yet completed.
  NABoolean cancelStarted_;   

  NABoolean cpuLimitExceeded_;

  // After fatal errors, this protects against more execution.
  NABoolean fatalError_;

  // This vector holds enable/disable status per trigger in a bit-wise manner
  // If needed, allocated, and copied to the ATP later.
  char *triggerStatusVector_;

  
  // Keep stream timeout - dynamic if set, else the static value
  Lng32 streamTimeout_;

  Int64 time_of_fetch_call_usec_; // Time when newOperation == TRUE
  
  // used by rowsets. See work() method.
  Lng32 lastQueueSize_;

  // Support for safe suspend:
  bool mayPinAudit_;
  bool mayLock_;

  // private methods
  
  // work proc to handle control messages to ESPs, maybe called directly
  // or triggered by fragDirEventHandler_
  ExWorkProcRetcode workOnFragDir();
  // static version
// warning elimination (removed "inline")
  static ExWorkProcRetcode sWorkOnFragDir(ex_tcb *tcb)
                        { return ((ex_root_tcb *) tcb)->workOnFragDir(); }

  void completeOutstandingCancelMsgs();

  // after scheduler returns bad error.
  Int32 fatal_error( ExExeStmtGlobals * glob, ComDiagsArea*& diagsArea,
                     NABoolean noFatalDiags = FALSE);

  // Make sure trans mode is compatible with Halloween/DP2 locks solution
  // and SUSPEND.  Returns -1 and populates diags area if error.
  Int32 checkTransBeforeExecute(ExTransaction *myTrans,
				ExMasterStmtGlobals *masterGlob, 
				ExMasterStats * rootStats,
				ComDiagsArea *& diags);

  // Let the cancel broker know this query is executing.
  void registerQuery(ComDiagsArea *&diagsArea, bool isReRegister = false);

  void populateCancelDiags( ComDiagsArea &diags);

};
#pragma warn(1319)  // warning elimination 

inline const ex_tcb* ex_root_tcb::getChild(Int32 pos) const
{
   ex_assert((pos >= 0), ""); 
   if (pos == 0)
      return tcbChild_;
   else
      return NULL;
}
#endif
