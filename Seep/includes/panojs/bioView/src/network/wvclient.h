/*******************************************************************************

  WVClient provides basic client interface for sending/receiving messages,
  it will be exchanging messages following protocol defined by WVMessenger  
  
  Author: Dima Fedorov Levit <dimin@dimin.net> <http://www.dimin.net/>
  Copyright (C) BioImage Informatics <www.bioimage.ucsb.edu>

  History:
    12/08/2006 18:09 - First creation
      
  ver: 1
        
*******************************************************************************/

#ifndef WV_CLIENT_H
#define WV_CLIENT_H

#include "wvmessenger.h"

class WVClient : public WVMessenger {
  Q_OBJECT

public:
  WVClient( );
  void connectToServer( const QString &hostName, int hostPort );
};

#endif // WV_CLIENT_H
