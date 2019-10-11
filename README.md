# stroot-forward-tracker

## STAR Forward Tracker Integration

### Getting started:

First, log into an rcas node.  Go to a suitable working directory, checkout
and build the code:

 > $ git clone https://github.com/klendathu2k/stroot-forward-tracker.git
 >
 > $ cd stroot-forward-tracker/
 >
 > $ git checkout dev
 >
 > $ cons

Next, you'll want to setup a quick and dirty test simulation--

 > $ starsim -w 0 -b StRoot/StgMaker/macros/testg.kumac nevents=10 ntrack=1 etamn=3.4 etamx=3.5 ptmn=4.9 ptmx=5.0

Which you can run using

 > $ root4star -q -b StRoot/StgMaker/macros/testg.C

### FAQ:

- Q: What is rcas?
- A: These are not the droids you are looking for.  Move along. 


