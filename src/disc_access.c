/*
 * File        disc_access.c
 * Author      Rodrigo Tacla Saad
 * Email       rodrigo.tacla.saad@gmail.com
 * Company:    UFSC
 * Created     on February 6, 2013, 10:31 AM
 *
 * LICENSE
 *
 * MIT License
 *
 * Copyright LAAS-CNRS / Vertics
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 * DESCRIPTION
 *
 * TODO
 * 
 */

#include "reset_define_includes.h"
#define STRINGLIB
#define ERRORLIB
#define ASSERTLIB
#define STDLIB
#define MAXMACRO
#include "disc_access.h"
#include "state_data.h"


pthread_mutex_t _mutex_disc_access;

int _disc_access_init = 0;

void disc_access_init(){
    if(!_disc_access_init){
        _disc_access_init = 1;
        pthread_mutex_init(&_mutex_disc_access, NULL);    
    }
}

void disc_access_lock(){
    pthread_mutex_lock(&_mutex_disc_access);
}

void disc_access_unlock(){
    pthread_mutex_unlock(&_mutex_disc_access);
}