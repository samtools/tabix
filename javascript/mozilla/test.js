/*
to run that test:

 * download mozilla xulrunner-sdk : https://developer.mozilla.org/en-US/docs/XULRunner
 * compile the tabix ldynamic ibrary using `make libtabix.so.1`
 * run the test: 


$ LD_LIBRARY_PATH=/path/to/xulrunner/bin:/path/to/tabix.dir \
	/path/to/xulrunner-sdk/bin/xpcshell -f test.js

*/

/** query a location  */
load("tabix.js");


var tabix=new TabixFile("../../example.gtf.gz");

print("#iteration using query(range)##################");

var iter=tabix.query("chr2:32800-35441");
while((line=iter.next())!=null)
 {
 print(line);
 }


iter=tabix.query("chr1:1873-2090");
while((line=iter.next())!=null)
 {
 print(line);
 }
iter.close();

print("#iteration using query()##################");
 /** get the 3 first lines */
iter=tabix.query();
for(var i=0; i< 3 && ((line=iter.next())!=null) ; ++i)
 {
 print(line);
 }
 
tabix.close();
