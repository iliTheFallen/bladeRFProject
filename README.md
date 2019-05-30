<h1> bladeRFProject </h1>

<h2> Communicating two bladeRF devices over TUN interfaces </h2>


    Copyright 2019 METU, Middle East Technical University, CENG
    
    This file is part of bladeRFProject.
   
    bladeRFProject is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
   
    bladeRFProject is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
   
    You should have received a copy of the GNU General Public License. Please contact Ilker GURCAN 
    for more info about licensing gurcan.ilker@gmail.com, or via github issues section.
    
    Authors:
      Ilker GURCAN
      Fehime Betul CAVDARLI
      Umay Ezgi KADAN

<p>
<ul>
 <li>An implementation of IP network stack for wireless communication</li>
 <li>Tested on Ubuntu 18.04 LTS platform</li>
 <li>Install all packages (including bladeRF library) using pybombs</li>
 <li>You should download and build liquidSDR separately</li>
 <li>Check out makefile for installation</li>
</ul>
</p>

<p>
  As for modules:
  
  
  |   Module  |  Purpose  | Build Cmd |
  |:---------:|:---------:|:---------:|
  | configTun | Used to create and configure TUN interfaces | make configTun |
  | receiver  | Receiver side application | make receiver |
  | transmitter  | Transmitter side application | make transmitter |
  
</p>
