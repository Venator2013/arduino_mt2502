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

#include "vmsystem.h"
#include "vmdatetime.h"
#include "LDateTime.h"

LDateTimeClass::LDateTimeClass()
{
}

int LDateTimeClass::getTime(datetimeInfo *time)
{
	return vm_time_get_date_time((vm_date_time_t *)time);//return value:-1:failed, time is null;  0:success
}

int LDateTimeClass::setTime(datetimeInfo *time)
{
	return vm_time_set_date_time((vm_date_time_t *)time);
}

int LDateTimeClass::getRtc(unsigned int *rtc)
{
	return vm_time_get_unix_time(rtc);//return value:-2:failed, rtc is null;  0:success
}

LDateTimeClass LDateTime;
