//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2014-2015, Lawrence Livermore National Security, LLC.
// 
// Produced at the Lawrence Livermore National Laboratory
// 
// LLNL-CODE-666778
// 
// All rights reserved.
// 
// This file is part of Conduit. 
// 
// For details, see: http://llnl.github.io/conduit/.
// 
// Please also read conduit/LICENSE
// 
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions are met:
// 
// * Redistributions of source code must retain the above copyright notice, 
//   this list of conditions and the disclaimer below.
// 
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the disclaimer (as noted below) in the
//   documentation and/or other materials provided with the distribution.
// 
// * Neither the name of the LLNS/LLNL nor the names of its contributors may
//   be used to endorse or promote products derived from this software without
//   specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL LAWRENCE LIVERMORE NATIONAL SECURITY,
// LLC, THE U.S. DEPARTMENT OF ENERGY OR CONTRIBUTORS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
// DAMAGES  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
// IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
// POSSIBILITY OF SUCH DAMAGE.
// 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

//-----------------------------------------------------------------------------
///
/// file: conduit_silo.hpp
///
//-----------------------------------------------------------------------------

#ifndef CONDUIT_SILO_HPP
#define CONDUIT_SILO_HPP

//-----------------------------------------------------------------------------
// external lib includes
//-----------------------------------------------------------------------------
#include <silo.h>

//-----------------------------------------------------------------------------
// conduit includes
//-----------------------------------------------------------------------------
#include "conduit_io.hpp"

//-----------------------------------------------------------------------------
// -- begin conduit:: --
//-----------------------------------------------------------------------------
namespace conduit
{

//-----------------------------------------------------------------------------
// -- begin conduit::io --
//-----------------------------------------------------------------------------
namespace io
{

//-----------------------------------------------------------------------------
void CONDUIT_IO_API silo_write(const  Node &node,
                               const std::string &path);

void CONDUIT_IO_API silo_read(const std::string &path,
                              Node &node);

//-----------------------------------------------------------------------------
void CONDUIT_IO_API silo_write(const  Node &node,
                               const std::string &file_path,
                               const std::string &silo_obj_path);

void CONDUIT_IO_API silo_read(const std::string &file_path,
                              const std::string &silo_obj_path,
                              Node &node);

//-----------------------------------------------------------------------------
void CONDUIT_IO_API silo_write(const  Node &node,
                               DBfile *dbfile,
                               const std::string &silo_obj_path);

void CONDUIT_IO_API silo_read(DBfile *dbfile,
                              const std::string &silo_obj_path,
                              Node &node);


//-----------------------------------------------------------------------------    
void CONDUIT_IO_API silo_mesh_write(Node &mesh,
                                    const std::string &path);

//-----------------------------------------------------------------------------
void CONDUIT_IO_API silo_mesh_write(Node &mesh,
                                    const std::string &file_path,
                                    const std::string &silo_obj_path);

//-----------------------------------------------------------------------------
void CONDUIT_IO_API silo_mesh_write(Node &mesh,
                                    DBfile *dbfile,
                                    const std::string &silo_obj_path);


}
//-----------------------------------------------------------------------------
// -- end conduit::io --
//-----------------------------------------------------------------------------


}
//-----------------------------------------------------------------------------
// -- end conduit:: --
//-----------------------------------------------------------------------------


#endif

