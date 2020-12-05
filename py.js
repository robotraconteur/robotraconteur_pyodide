var Module=typeof pyodide._module!=="undefined"?pyodide._module:{};Module.checkABI(1);if(!Module.expectedDataFileDownloads){Module.expectedDataFileDownloads=0;Module.finishedDataFileDownloads=0}Module.expectedDataFileDownloads++;(function(){var loadPackage=function(metadata){var PACKAGE_PATH;if(typeof window==="object"){PACKAGE_PATH=window["encodeURIComponent"](window.location.pathname.toString().substring(0,window.location.pathname.toString().lastIndexOf("/"))+"/")}else if(typeof location!=="undefined"){PACKAGE_PATH=encodeURIComponent(location.pathname.toString().substring(0,location.pathname.toString().lastIndexOf("/"))+"/")}else{throw"using preloaded data can only be done on a web page or in a web worker"}var PACKAGE_NAME="py.data";var REMOTE_PACKAGE_BASE="py.data";if(typeof Module["locateFilePackage"]==="function"&&!Module["locateFile"]){Module["locateFile"]=Module["locateFilePackage"];err("warning: you defined Module.locateFilePackage, that has been renamed to Module.locateFile (using your locateFilePackage for now)")}var REMOTE_PACKAGE_NAME=Module["locateFile"]?Module["locateFile"](REMOTE_PACKAGE_BASE,""):REMOTE_PACKAGE_BASE;var REMOTE_PACKAGE_SIZE=metadata.remote_package_size;var PACKAGE_UUID=metadata.package_uuid;function fetchRemotePackage(packageName,packageSize,callback,errback){var xhr=new XMLHttpRequest;xhr.open("GET",packageName,true);xhr.responseType="arraybuffer";xhr.onprogress=function(event){var url=packageName;var size=packageSize;if(event.total)size=event.total;if(event.loaded){if(!xhr.addedTotal){xhr.addedTotal=true;if(!Module.dataFileDownloads)Module.dataFileDownloads={};Module.dataFileDownloads[url]={loaded:event.loaded,total:size}}else{Module.dataFileDownloads[url].loaded=event.loaded}var total=0;var loaded=0;var num=0;for(var download in Module.dataFileDownloads){var data=Module.dataFileDownloads[download];total+=data.total;loaded+=data.loaded;num++}total=Math.ceil(total*Module.expectedDataFileDownloads/num);if(Module["setStatus"])Module["setStatus"]("Downloading data... ("+loaded+"/"+total+")")}else if(!Module.dataFileDownloads){if(Module["setStatus"])Module["setStatus"]("Downloading data...")}};xhr.onerror=function(event){throw new Error("NetworkError for: "+packageName)};xhr.onload=function(event){if(xhr.status==200||xhr.status==304||xhr.status==206||xhr.status==0&&xhr.response){var packageData=xhr.response;callback(packageData)}else{throw new Error(xhr.statusText+" : "+xhr.responseURL)}};xhr.send(null)}function handleError(error){console.error("package error:",error)}var fetchedCallback=null;var fetched=Module["getPreloadedPackage"]?Module["getPreloadedPackage"](REMOTE_PACKAGE_NAME,REMOTE_PACKAGE_SIZE):null;if(!fetched)fetchRemotePackage(REMOTE_PACKAGE_NAME,REMOTE_PACKAGE_SIZE,function(data){if(fetchedCallback){fetchedCallback(data);fetchedCallback=null}else{fetched=data}},handleError);function runWithFS(){function assert(check,msg){if(!check)throw msg+(new Error).stack}Module["FS_createPath"]("/","lib",true,true);Module["FS_createPath"]("/lib","python3.8",true,true);Module["FS_createPath"]("/lib/python3.8","site-packages",true,true);Module["FS_createPath"]("/lib/python3.8/site-packages","py",true,true);Module["FS_createPath"]("/lib/python3.8/site-packages/py","_process",true,true);Module["FS_createPath"]("/lib/python3.8/site-packages/py","_code",true,true);Module["FS_createPath"]("/lib/python3.8/site-packages/py","_vendored_packages",true,true);Module["FS_createPath"]("/lib/python3.8/site-packages/py","_path",true,true);Module["FS_createPath"]("/lib/python3.8/site-packages/py","_log",true,true);Module["FS_createPath"]("/lib/python3.8/site-packages/py","_io",true,true);Module["FS_createPath"]("/lib/python3.8/site-packages","py-1.8.0-py3.8.egg-info",true,true);function DataRequest(start,end,audio){this.start=start;this.end=end;this.audio=audio}DataRequest.prototype={requests:{},open:function(mode,name){this.name=name;this.requests[name]=this;Module["addRunDependency"]("fp "+this.name)},send:function(){},onload:function(){var byteArray=this.byteArray.subarray(this.start,this.end);this.finish(byteArray)},finish:function(byteArray){var that=this;Module["FS_createPreloadedFile"](this.name,null,byteArray,true,true,function(){Module["removeRunDependency"]("fp "+that.name)},function(){if(that.audio){Module["removeRunDependency"]("fp "+that.name)}else{err("Preloading file "+that.name+" failed")}},false,true);this.requests[this.name]=null}};function processPackageData(arrayBuffer){Module.finishedDataFileDownloads++;assert(arrayBuffer,"Loading data file failed.");assert(arrayBuffer instanceof ArrayBuffer,"bad input to processPackageData");var byteArray=new Uint8Array(arrayBuffer);var curr;var compressedData={data:null,cachedOffset:153982,cachedIndexes:[-1,-1],cachedChunks:[null,null],offsets:[0,1135,2260,3432,4653,6017,7286,8441,9811,11009,11861,12916,14073,15180,16429,17872,19037,19948,20735,21660,22608,23607,24786,26144,27390,28552,29705,30922,32175,33311,34429,35486,36656,37776,38892,39824,40765,42176,43560,44681,45978,47108,48011,48825,49705,50882,51959,53111,54296,55600,56886,58223,59575,60770,61826,62961,63970,64968,66279,67375,68395,69675,70713,71983,73370,74656,75631,76645,77860,79035,80145,81338,82540,83737,84961,86163,87233,88408,89392,90576,91705,92768,93843,94911,96169,97367,98499,99571,100809,101955,103093,104141,105358,106702,107735,108887,110043,111254,112549,113725,114886,116051,117254,118262,119514,120736,121917,123071,124073,125223,126151,127394,128792,129911,131159,132349,133462,134659,135971,137352,138424,139605,140824,141834,142851,144043,145262,146459,147706,148536,149666,150998,151770,152879,153981],sizes:[1135,1125,1172,1221,1364,1269,1155,1370,1198,852,1055,1157,1107,1249,1443,1165,911,787,925,948,999,1179,1358,1246,1162,1153,1217,1253,1136,1118,1057,1170,1120,1116,932,941,1411,1384,1121,1297,1130,903,814,880,1177,1077,1152,1185,1304,1286,1337,1352,1195,1056,1135,1009,998,1311,1096,1020,1280,1038,1270,1387,1286,975,1014,1215,1175,1110,1193,1202,1197,1224,1202,1070,1175,984,1184,1129,1063,1075,1068,1258,1198,1132,1072,1238,1146,1138,1048,1217,1344,1033,1152,1156,1211,1295,1176,1161,1165,1203,1008,1252,1222,1181,1154,1002,1150,928,1243,1398,1119,1248,1190,1113,1197,1312,1381,1072,1181,1219,1010,1017,1192,1219,1197,1247,830,1130,1332,772,1109,1102,1],successes:[1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0]};compressedData.data=byteArray;assert(typeof Module.LZ4==="object","LZ4 not present - was your app build with  -s LZ4=1  ?");Module.LZ4.loadPackage({metadata:metadata,compressedData:compressedData});Module["removeRunDependency"]("datafile_py.data")}Module["addRunDependency"]("datafile_py.data");if(!Module.preloadResults)Module.preloadResults={};Module.preloadResults[PACKAGE_NAME]={fromCache:false};if(fetched){processPackageData(fetched);fetched=null}else{fetchedCallback=processPackageData}}if(Module["calledRun"]){runWithFS()}else{if(!Module["preRun"])Module["preRun"]=[];Module["preRun"].push(runWithFS)}};loadPackage({files:[{filename:"/lib/python3.8/site-packages/py/_builtin.py",start:0,end:4021,audio:0},{filename:"/lib/python3.8/site-packages/py/_error.py",start:4021,end:6938,audio:0},{filename:"/lib/python3.8/site-packages/py/_std.py",start:6938,end:7569,audio:0},{filename:"/lib/python3.8/site-packages/py/_xmlgen.py",start:7569,end:15933,audio:0},{filename:"/lib/python3.8/site-packages/py/test.py",start:15933,end:16155,audio:0},{filename:"/lib/python3.8/site-packages/py/__init__.py",start:16155,end:22177,audio:0},{filename:"/lib/python3.8/site-packages/py/_version.py",start:22177,end:22293,audio:0},{filename:"/lib/python3.8/site-packages/py/__metainfo.py",start:22293,end:22348,audio:0},{filename:"/lib/python3.8/site-packages/py/_process/forkedfunc.py",start:22348,end:26040,audio:0},{filename:"/lib/python3.8/site-packages/py/_process/killproc.py",start:26040,end:26688,audio:0},{filename:"/lib/python3.8/site-packages/py/_process/cmdexec.py",start:26688,end:28502,audio:0},{filename:"/lib/python3.8/site-packages/py/_process/__init__.py",start:28502,end:28542,audio:0},{filename:"/lib/python3.8/site-packages/py/_code/_assertionold.py",start:28542,end:46411,audio:0},{filename:"/lib/python3.8/site-packages/py/_code/code.py",start:46411,end:73903,audio:0},{filename:"/lib/python3.8/site-packages/py/_code/_py2traceback.py",start:73903,end:76668,audio:0},{filename:"/lib/python3.8/site-packages/py/_code/__init__.py",start:76668,end:76714,audio:0},{filename:"/lib/python3.8/site-packages/py/_code/assertion.py",start:76714,end:79888,audio:0},{filename:"/lib/python3.8/site-packages/py/_code/_assertionnew.py",start:79888,end:91338,audio:0},{filename:"/lib/python3.8/site-packages/py/_code/source.py",start:91338,end:105388,audio:0},{filename:"/lib/python3.8/site-packages/py/_vendored_packages/apipkg.py",start:105388,end:111808,audio:0},{filename:"/lib/python3.8/site-packages/py/_vendored_packages/iniconfig.py",start:111808,end:117016,audio:0},{filename:"/lib/python3.8/site-packages/py/_vendored_packages/__init__.py",start:117016,end:117016,audio:0},{filename:"/lib/python3.8/site-packages/py/_path/svnurl.py",start:117016,end:131731,audio:0},{filename:"/lib/python3.8/site-packages/py/_path/common.py",start:131731,end:146357,audio:0},{filename:"/lib/python3.8/site-packages/py/_path/cacheutil.py",start:146357,end:149690,audio:0},{filename:"/lib/python3.8/site-packages/py/_path/local.py",start:149690,end:186387,audio:0},{filename:"/lib/python3.8/site-packages/py/_path/svnwc.py",start:186387,end:230212,audio:0},{filename:"/lib/python3.8/site-packages/py/_path/__init__.py",start:230212,end:230244,audio:0},{filename:"/lib/python3.8/site-packages/py/_log/warning.py",start:230244,end:232809,audio:0},{filename:"/lib/python3.8/site-packages/py/_log/__init__.py",start:232809,end:232883,audio:0},{filename:"/lib/python3.8/site-packages/py/_log/log.py",start:232883,end:238886,audio:0},{filename:"/lib/python3.8/site-packages/py/_io/saferepr.py",start:238886,end:241369,audio:0},{filename:"/lib/python3.8/site-packages/py/_io/terminalwriter.py",start:241369,end:256029,audio:0},{filename:"/lib/python3.8/site-packages/py/_io/__init__.py",start:256029,end:256058,audio:0},{filename:"/lib/python3.8/site-packages/py/_io/capture.py",start:256058,end:267698,audio:0},{filename:"/lib/python3.8/site-packages/py-1.8.0-py3.8.egg-info/SOURCES.txt",start:267698,end:271310,audio:0},{filename:"/lib/python3.8/site-packages/py-1.8.0-py3.8.egg-info/not-zip-safe",start:271310,end:271311,audio:0},{filename:"/lib/python3.8/site-packages/py-1.8.0-py3.8.egg-info/top_level.txt",start:271311,end:271314,audio:0},{filename:"/lib/python3.8/site-packages/py-1.8.0-py3.8.egg-info/PKG-INFO",start:271314,end:274432,audio:0},{filename:"/lib/python3.8/site-packages/py-1.8.0-py3.8.egg-info/dependency_links.txt",start:274432,end:274433,audio:0}],remote_package_size:158078,package_uuid:"cbbb4336-a30f-4577-9bcd-8603b043bcba"})})();