/*
  Copyright (c) 2014 MediaTek Inc.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License..

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
   See the GNU Lesser General Public License for more details.
*/

#include "Arduino.h"
#include "vmgsm_sim.h"
#include "CheckSIM.h"

boolean simIsCheck(void* user_data)
{
	VMBOOL check = vm_gsm_sim_has_card();
  *(boolean*)user_data = check;
  return true;
}
