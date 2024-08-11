#include <iostream>
#include "oci.h"
#include <cstring>

// -pthread
/*句柄分配*/
ACIEnv                        *m_penv = NULL;                 /*环境句柄*/
ACIError              *m_perr = NULL;                 /*错误句柄*/
ACIServer             *m_psrv = NULL;                 /*服务器句柄*/
ACISvcCtx             *m_psvc = NULL;                 /*服务上下文句柄*/
ACISession            *m_pses = NULL;                 /*会话句柄*/
ACIStmt                       *m_pstmt = NULL;                /*语句句柄*/
ACIBind                       *m_bnd = NULL;                  /*绑定句柄*/
ACIDefine             *m_def = NULL;                  /*定义句柄*/

typedef struct
{
      int sID;
      int sCol1;
} Record;


int main()
{
      ACIEnv *m_penv = NULL;/*环境句柄*/
      ACIError *m_perr = NULL;/*错误句柄*/
      ACIServer *m_psrv = NULL;/*服务器句柄*/
      ACISvcCtx *m_psvc = NULL;/*服务上下文句柄*/
      sword r = ACI_SUCCESS;
      char *m_dblink = (char *)"localhost:2003/DATEBASE1";
      char *m_dbuser = (char *)"sysdba";
      char *m_dbpwd = (char *)"szoscar55";
      //初始化环境句柄
      r = ACIInitialize(ACI_DEFAULT, NULL, NULL, NULL, NULL);
      r = ACIEnvInit(&m_penv, ACI_DEFAULT, 0, 0);

      //分配并初始化各类应用句柄
      r = ACIHandleAlloc(m_penv, (void**)&m_perr, ACI_HTYPE_ERROR, 0, 0);
      r = ACIHandleAlloc(m_penv, (void**)&m_psrv, ACI_HTYPE_SERVER, 0, 0);
      r = ACIHandleAlloc(m_penv, (void**)&m_psvc, ACI_HTYPE_SVCCTX, 0, 0);
      //连接数据库
      r = ACILogon(m_penv, m_perr, &m_psvc, (const OraText *)m_dbuser, strlen(m_dbuser), (const OraText *)m_dbpwd, strlen(m_dbpwd), (const OraText *)m_dblink, strlen(m_dblink));
      printf("Connect OK?");

      char ssql[] = "select version();";
      r = ACIStmtPrepare(m_pstmt, m_perr, (OraText*)ssql, strlen(ssql), ACI_HTYPE_ERROR, ACI_DEFAULT);

      /*----------按位置进行定义-------------*/
      r = ACIDefineByPos(m_pstmt, &m_def, m_perr, 1, (void*)&rec.sID, sizeof(rec.sID), SQLT_INT, (void*)0, 0, 0, ACI_DEFAULT);
      r = ACIDefineByPos(m_pstmt, &m_def, m_perr, 2, (void *)&rec.sCol1, sizeof(rec.sCol1), SQLT_INT, (void*)0, 0, 0, ACI_DEFAULT);

      /*----------执行SQL语句-------------*/
      r = ACIStmtExecute(m_psvc, m_pstmt, m_perr, 0, 0, 0, 0, ACI_DEFAULT);

      /*----------结果获取-------------*/
      while (((r = ACIStmtFetch(m_pstmt, m_perr, 1, ACI_FETCH_NEXT, ACI_DEFAULT)) != ACI_NO_DATA))
      {
            printf("%d, %d \n", rec.sID, rec.sCol1);
      }

      r = ACILogoff(m_psvc, m_perr);
      printf("Logout");
}
