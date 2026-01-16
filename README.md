# SOLAr-sim

**SOLAr-sim** (historically also called `G4SOLAr`) is a software package
providing a Geant4-based simulation of a liquid argon time projection chamber (LArTPC)
optimized for the study of low-energy events. It is part of the
[SoLAr project](https://solar-project.web.cern.ch/), a research initiative
focused on the development of a novel LArTPC concept using a pixelated anode
integrating photosensitive devices (SiPMs) for the detection of scintillation light.

## Prerequisites

- **Core:** `Geant4` `v11.0` and `v11.1.pXX`, **compiled without `MULTI_THREAD` support**, 
  `ROOT`
  and respective dependencies (`cmake`, `g++`, `gcc`)
- **Physics**: `SOLAr-sim` integrates by default the `G4CASCADE` package
    for the simulation of gamma-ray cascades following neutron captures
    (L. Weimer, M. Lai, E. Ellingwood & S. Westerdale, arXiv:2408.02774 [physics.comp-ph] 2024, 
   [github repo](https://github.com/UCRDarkMatter/CASCADE)). 
   Users can also choose to not use this package by setting the 
   `SLAR_USE_G4CASCADE` flag to `OFF` in the `cmake` command line.
- **Generators:** `SOLAr-sim` integrates some external events generators that
  are relevant for the physics goal of the project. 
  * **MARLEY**: Low-energy neutrino interactions in LAr 
  (S. Gardiner, _Comput.Phys.Commun._ 269:108123 (2021). arXiv:2101.11867)
  * **BxDecay0**: Generic radioactive decay generator, with the possibility 
  of producing neutrinoless *ββ*-decay final states
  (F. Mauger & V. Tretyak, [github repo](https://github.com/BxCppDev/bxdecay0))
  * **CRY**: [optional] Cosmic-ray shower generator
  (C. Hagmann, D. Lange, J. Verbeke & D. Wright, LLNL, [landing page](https://nuclear.llnl.gov/simulation/main.html))
  * **RadSrc**: [optional] Generator reproducing the gamma ray spectrum of composite sources 
  (L. Hiller, T. Gosnell, J. Gronberg & D. Wright, LLNL [landing page](https://nuclear.llnl.gov/simulation/main.html))
- **Utilities**: `SOLAr-sim` uses the `RapidJSON` package to parse configuration 
  files formatted according to the `json` standard. 
    
  The `SOLAr-sim` package includes a convenient script 
  to automatically download, build and install the required external dependencies
  (see [External dependencies installation and configuration](G4SOLAr/extern/README_EXTERNALS.md)). 
  Optional dependencies such as `CRY` and `RadRsc` must be installed by the user, 
  and their respective environment variables (`CRYHOME` and `RADSRC_HOME`) 
  must be set before compiling the project.
  
## Download and build the project

Please note that the procedure to build the code have some subtle differences 
whether one is installing the simulation on a generic machine
such as the CERN computing environment. Here we describe the installation for a 
generic Unix system.

### Step 1 - Download the project from GitHub and install dependencies
(If you prefer ssh key authentication change the repository url accordingly)
```bash
$ git clone https://github.com/SoLAr-Neutrinos/SOLAr-sim.git
$ git submodule update --init --recursive
```
If the project dependencies are not yet installed, follow the instructions
on [this page](./G4SOLAr/extern/README_EXTERNALS.md). The only submodule invoked 
is a styling package for the doxygen documentation, and it is not strictly
required.

### Step 2 - Build and install
Create a build and install directory, then build and install the project
```bash
$ mkdir build install && cd build 
$ cmake -DCMAKE_INSTALL_PREFIX=../install [opts...] ../G4SOLAr
$ cmake --build . -j $(nproc) 
$ cmake --install . 
```
The project will search for the external dependencies in the 
`G4SOLAR_EXT_DIR` (by default set to `G4SOLAr/extern/install`). You can 
specify a specific installation directory by setting it in the `cmake`
command line (`-DG4SOLAR_EXT_DIR=/my/g4solar_ext/path`). 

### Step 3 - Run SOLAr-sim

It is possible to run the simulation directly from the installation folder, but it
is advised to add the build directory to the executable PATH to be able to run 
the simulation more flexibly on your machine
```bash
$ cd install
$ export PATH=${PWD}:${PATH}/bin
```

The `solar_sim` executable can take the following inputs:
```bash
solar_sim      
           [-g | --geometry geometry_cfg_file]      #<< Geometry description
           [-p | --materials material_db_file]      #<< Material definition table
           [-x | --generator generator_config_file] #<< Configure the primary generators
           [-m | --macro macro_file]                #<< Geant4 mac file
           [-d | --output_dir output_dir]           #<< set output directory
           [-o | --output output_file]              #<< set output file name
           [-l | --physics_list modular_phys_list]  #<< set basic physics list (default is FTFP_BERT_HP)
           [-r | --seed user_seed]                  #<< User defined seed
           [-b | --bias particle <process_list> bias_factor] #<< apply a scaling factor to a given particle (process) cross section
           [-h | --help print usage]    
```

Not all inputs are mandatory. The only required inputs is the
`geometry` configuration file, which describes the geometry of the 
experimental setup. 
Passing only the `geometry` input, the application will open the user interface 
(if available) and will stay idle until the user passes any other command.
By default, the application will look for the material definitions
in the `assets/materials/materials_db.json` file. One can set a different
material database by passing the `-p` or `--materials` option.

Primary generators are configured in dedicated `json` configuration files
passed with the `-x` or `--generator` option or by using the 
`SLAr/gen/configure` interactive command.

The `--macro` input is the configuration file for the 
Geant4 run. There one can specify the type of generated events, 
their number, the output file, etc. A collection of examples can 
be found in the `macros/` folder. The commands defined in the messenger
classes are briefly commented in the macro files. 

## Documentation

You can locally build the `SOLAr-sim` documentation using `doxygen`.
By running the following command in the `docs` folder, you will generate
the documentation in the `html` format, available in the `docs/html` folder.
```bash
$ cd docs
$ doxygen solar-sim-doc.doxy
```
You can open the `index.html` file in your browser to navigate the documentation.


## Using SOLAr-sim

SOLAr-sim is distributed under MIT license. If you use it for your research, please
consider citing this repository and the SoLAr project in your publications.

```
@online{SOLAr-sim,
  author       = "{D.\ Guffanti et al.\ (SoLAr Collaboration)}",
  title        = {SOLAr-sim: Simulation framework for the SoLAr experiment},
  year         = {2025},
  url          = {https://github.com/SoLAr-Neutrinos/SOLAr-sim},
}
```

### Computing environment support

#### CERN
See the documentation of [**lx-solar**](https://github.com/SoLAr-Neutrinos/lx-solar), 
a set of scripts and utilities setting the SoLAr environment on lxplus.


