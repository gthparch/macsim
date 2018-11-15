#Macsim [![Build Status](https://travis-ci.org/gthparch/macsim.svg?branch=master)](https://travis-ci.org/gthparch/macsim)
#### THIS BRANCH IS FOR INTEL GEN GPUs.

## Introduction

* MacSim is a heterogeneous architecture timing model simulator developed from Georgia Institute of Technology.
* It is trace-driven and simulates Intel GEN GPU instructions (We use GT-Pin to generate traces).
* We've characterised the performance of Intel GEN GPUs with MacSim. Please refer to the following paper for more detailed information. [Performance Characterisation and Simulation of Intel's Integrated GPU Architecture (ISPASS'18)](http://comparch.gatech.edu/hparch/papers/gera_ispass18.pdf)
* We're currently working on a flexible memory hierarchy that can be applicable for both Intel GEN GPUs and NVIDIA GPUs.


## Documentation

Please see [MacSim documentation file](http://macsim.googlecode.com/files/macsim.pdf) for more detailed descriptions.


## Download

* You can download the latest copy from our git repository.

```
  git clone https://github.com/gthparch/macsim/tree/intel_gpu
```


## People

* Prof. Hyesoon Kim (Project Leader) at Georgia Tech 
Hparch research group 
(http://comparch.gatech.edu/hparch/people.hparch) 


## Mailing list

If you have a question, feel free to send an email to macsim-dev@googlegroups.com.


## Tutorial

* We had a tutorial in HPCA-2012. Please visit [here](http://comparch.gatech.edu/hparch/OcelotMacsim_tutorial.html) for the slides.
* We had a tutorial in ISCA-2012, Please visit [here](http://comparch.gatech.edu/hparch/isca12_gt.html) for the slides.


## SST+MacSim

* Here are two example configurations of SST+MacSim.
  * A multi-socket system with cache coherence model: ![](http://comparch.gatech.edu/hparch/images/sst+macsim_conf_1.png)
  * A CPU+GPU heterogeneous system with shared memory: ![](http://comparch.gatech.edu/hparch/images/sst+macsim_conf_2.png)
