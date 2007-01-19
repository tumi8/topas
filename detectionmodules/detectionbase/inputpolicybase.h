/**************************************************************************/
/*    Copyright (C) 2005-2007 Lothar Braun <mail@lobraun.de>              */
/*                                                                        */
/*    This library is free software; you can redistribute it and/or       */
/*    modify it under the terms of the GNU Lesser General Public          */
/*    License as published by the Free Software Foundation; either        */
/*    version 2.1 of the License, or (at your option) any later version.  */
/*                                                                        */
/*    This library is distributed in the hope that it will be useful,     */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of      */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU   */
/*    Lesser General Public License for more details.                     */
/*                                                                        */
/*    You should have received a copy of the GNU Lesser General Public    */
/*    License along with this library; if not, write to the Free Software  */
/*    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA    */
/**************************************************************************/

#ifndef _INPUTPOLICYBASE_H_
#define _INPUTPOLICYBASE_H_


#include "detectcallbacks.h"


#include <commonutils/mutex.h>


#include <pthread.h>


#include <vector>


/**
 * Base class for all notification objects. The notification objects are
 * responsible for informing the detection module about new
 * data.
 */
class InputNotificationBase {
public:
        /**
         * Inherited classes should override (hide) this method.
         * The mehtod will be called whenever the detection module is ready
         * for new data.
         * @return > 0 - new data available
         *         0 - no further data (connection closed or something like that)
         */
        int wait() const { return 0; }

        /**
         * Inherited classes should override (hide) this method.
         * The method will be called whenever an detection module finished processing
         * data.
         */
        int notify() const { return 0; }
};


/**
 * Base class for all input policies. The input policies are responsible
 * for importing flow data into the detection module.
 */
template <
        class Notifier,
        class Storage
>
class InputPolicyBase 
{
public:
        InputPolicyBase() 
        {
        }
        ~InputPolicyBase() 
        {
	}

        /**
         * Blocks until new data is ready for import.
         * @return > 0 - new data available
         *         0 - no further data (connection closed or something like that)
         */
        int wait() const
        {
                return notifier.wait();
        }

        /**
         * Imports data into Storage.
         */
        void importToStorage() 
        {
                
        }

        /**
         * Informs collector, that all packets are parsed.
         */
        int notify() const
        {
                return notifier.notify();
        }

	/**
	 * Returns notifier element.
	 */
        Notifier& getNotifier() 
        {
                return notifier;
        }

	/**
	 *   Returns a storage element
	 */
        Storage* getStorage()
        {
        }

private:
        Notifier notifier;
};

#endif
