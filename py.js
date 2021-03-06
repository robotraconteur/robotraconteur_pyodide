var Module=typeof pyodide._module!=="undefined"?pyodide._module:{};if(!Module.expectedDataFileDownloads){Module.expectedDataFileDownloads=0;Module.finishedDataFileDownloads=0}Module.expectedDataFileDownloads++;(function(){var loadPackage=function(metadata){var PACKAGE_PATH;if(typeof window==="object"){PACKAGE_PATH=window["encodeURIComponent"](window.location.pathname.toString().substring(0,window.location.pathname.toString().lastIndexOf("/"))+"/")}else if(typeof location!=="undefined"){PACKAGE_PATH=encodeURIComponent(location.pathname.toString().substring(0,location.pathname.toString().lastIndexOf("/"))+"/")}else{throw"using preloaded data can only be done on a web page or in a web worker"}var PACKAGE_NAME="py.data";var REMOTE_PACKAGE_BASE="py.data";if(typeof Module["locateFilePackage"]==="function"&&!Module["locateFile"]){Module["locateFile"]=Module["locateFilePackage"];err("warning: you defined Module.locateFilePackage, that has been renamed to Module.locateFile (using your locateFilePackage for now)")}var REMOTE_PACKAGE_NAME=Module["locateFile"]?Module["locateFile"](REMOTE_PACKAGE_BASE,""):REMOTE_PACKAGE_BASE;var REMOTE_PACKAGE_SIZE=metadata.remote_package_size;var PACKAGE_UUID=metadata.package_uuid;function fetchRemotePackage(packageName,packageSize,callback,errback){var xhr=new XMLHttpRequest;xhr.open("GET",packageName,true);xhr.responseType="arraybuffer";xhr.onprogress=function(event){var url=packageName;var size=packageSize;if(event.total)size=event.total;if(event.loaded){if(!xhr.addedTotal){xhr.addedTotal=true;if(!Module.dataFileDownloads)Module.dataFileDownloads={};Module.dataFileDownloads[url]={loaded:event.loaded,total:size}}else{Module.dataFileDownloads[url].loaded=event.loaded}var total=0;var loaded=0;var num=0;for(var download in Module.dataFileDownloads){var data=Module.dataFileDownloads[download];total+=data.total;loaded+=data.loaded;num++}total=Math.ceil(total*Module.expectedDataFileDownloads/num);if(Module["setStatus"])Module["setStatus"]("Downloading data... ("+loaded+"/"+total+")")}else if(!Module.dataFileDownloads){if(Module["setStatus"])Module["setStatus"]("Downloading data...")}};xhr.onerror=function(event){throw new Error("NetworkError for: "+packageName)};xhr.onload=function(event){if(xhr.status==200||xhr.status==304||xhr.status==206||xhr.status==0&&xhr.response){var packageData=xhr.response;callback(packageData)}else{throw new Error(xhr.statusText+" : "+xhr.responseURL)}};xhr.send(null)}function handleError(error){console.error("package error:",error)}var fetchedCallback=null;var fetched=Module["getPreloadedPackage"]?Module["getPreloadedPackage"](REMOTE_PACKAGE_NAME,REMOTE_PACKAGE_SIZE):null;if(!fetched)fetchRemotePackage(REMOTE_PACKAGE_NAME,REMOTE_PACKAGE_SIZE,function(data){if(fetchedCallback){fetchedCallback(data);fetchedCallback=null}else{fetched=data}},handleError);function runWithFS(){function assert(check,msg){if(!check)throw msg+(new Error).stack}Module["FS_createPath"]("/","lib",true,true);Module["FS_createPath"]("/lib","python3.8",true,true);Module["FS_createPath"]("/lib/python3.8","site-packages",true,true);Module["FS_createPath"]("/lib/python3.8/site-packages","py",true,true);Module["FS_createPath"]("/lib/python3.8/site-packages/py","_process",true,true);Module["FS_createPath"]("/lib/python3.8/site-packages/py","_code",true,true);Module["FS_createPath"]("/lib/python3.8/site-packages/py","_vendored_packages",true,true);Module["FS_createPath"]("/lib/python3.8/site-packages/py/_vendored_packages","apipkg-1.4.dist-info",true,true);Module["FS_createPath"]("/lib/python3.8/site-packages/py/_vendored_packages","iniconfig-1.0.0.dist-info",true,true);Module["FS_createPath"]("/lib/python3.8/site-packages/py","_path",true,true);Module["FS_createPath"]("/lib/python3.8/site-packages/py","_log",true,true);Module["FS_createPath"]("/lib/python3.8/site-packages/py","_io",true,true);Module["FS_createPath"]("/lib/python3.8/site-packages","py-1.9.0-py3.8.egg-info",true,true);function DataRequest(start,end,audio){this.start=start;this.end=end;this.audio=audio}DataRequest.prototype={requests:{},open:function(mode,name){this.name=name;this.requests[name]=this;Module["addRunDependency"]("fp "+this.name)},send:function(){},onload:function(){var byteArray=this.byteArray.subarray(this.start,this.end);this.finish(byteArray)},finish:function(byteArray){var that=this;Module["FS_createPreloadedFile"](this.name,null,byteArray,true,true,function(){Module["removeRunDependency"]("fp "+that.name)},function(){if(that.audio){Module["removeRunDependency"]("fp "+that.name)}else{err("Preloading file "+that.name+" failed")}},false,true);this.requests[this.name]=null}};function processPackageData(arrayBuffer){Module.finishedDataFileDownloads++;assert(arrayBuffer,"Loading data file failed.");assert(arrayBuffer instanceof ArrayBuffer,"bad input to processPackageData");var byteArray=new Uint8Array(arrayBuffer);var curr;var compressedData={data:null,cachedOffset:171928,cachedIndexes:[-1,-1],cachedChunks:[null,null],offsets:[0,1072,2134,2532,3686,4721,5845,7048,8062,8983,9966,11232,12514,13874,14968,16230,17607,18501,19390,20287,21113,22260,23375,24638,26056,27213,28117,28913,29848,30788,31783,32955,34304,35548,36714,37864,39092,40341,41479,42590,43659,44835,45959,47067,47991,48933,50351,51719,52836,54121,55263,56193,57012,57898,59065,60123,61277,62442,63749,65056,66395,67740,68924,70006,71142,72147,73160,74680,76163,77723,79288,80605,82060,83519,84706,85898,86917,88146,89128,90590,91808,92983,94090,95136,96404,97513,98742,99827,101060,102322,103465,104675,105756,106860,107900,109094,110241,111134,112250,113376,114657,115705,116912,117922,119142,120263,121233,122473,123737,124971,126015,127251,128487,129695,130903,132017,133171,134257,135542,136627,137883,139015,140364,141442,142411,143405,144350,145626,146887,148077,149389,150462,151568,152823,154179,155442,156695,157928,158974,160019,160952,162255,163389,164685,165877,166734,167856,169104,169912,171103],sizes:[1072,1062,398,1154,1035,1124,1203,1014,921,983,1266,1282,1360,1094,1262,1377,894,889,897,826,1147,1115,1263,1418,1157,904,796,935,940,995,1172,1349,1244,1166,1150,1228,1249,1138,1111,1069,1176,1124,1108,924,942,1418,1368,1117,1285,1142,930,819,886,1167,1058,1154,1165,1307,1307,1339,1345,1184,1082,1136,1005,1013,1520,1483,1560,1565,1317,1455,1459,1187,1192,1019,1229,982,1462,1218,1175,1107,1046,1268,1109,1229,1085,1233,1262,1143,1210,1081,1104,1040,1194,1147,893,1116,1126,1281,1048,1207,1010,1220,1121,970,1240,1264,1234,1044,1236,1236,1208,1208,1114,1154,1086,1285,1085,1256,1132,1349,1078,969,994,945,1276,1261,1190,1312,1073,1106,1255,1356,1263,1253,1233,1046,1045,933,1303,1134,1296,1192,857,1122,1248,808,1191,825],successes:[1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]};compressedData.data=byteArray;assert(typeof Module.LZ4==="object","LZ4 not present - was your app build with  -s LZ4=1  ?");Module.LZ4.loadPackage({metadata:metadata,compressedData:compressedData});Module["removeRunDependency"]("datafile_py.data")}Module["addRunDependency"]("datafile_py.data");if(!Module.preloadResults)Module.preloadResults={};Module.preloadResults[PACKAGE_NAME]={fromCache:false};if(fetched){processPackageData(fetched);fetched=null}else{fetchedCallback=processPackageData}}if(Module["calledRun"]){runWithFS()}else{if(!Module["preRun"])Module["preRun"]=[];Module["preRun"].push(runWithFS)}};loadPackage({files:[{filename:"/lib/python3.8/site-packages/py/xml.pyi",start:0,end:787,audio:0},{filename:"/lib/python3.8/site-packages/py/io.pyi",start:787,end:6064,audio:0},{filename:"/lib/python3.8/site-packages/py/iniconfig.pyi",start:6064,end:7410,audio:0},{filename:"/lib/python3.8/site-packages/py/_builtin.py",start:7410,end:11431,audio:0},{filename:"/lib/python3.8/site-packages/py/_error.py",start:11431,end:14348,audio:0},{filename:"/lib/python3.8/site-packages/py/path.pyi",start:14348,end:21516,audio:0},{filename:"/lib/python3.8/site-packages/py/_std.py",start:21516,end:22184,audio:0},{filename:"/lib/python3.8/site-packages/py/_xmlgen.py",start:22184,end:30548,audio:0},{filename:"/lib/python3.8/site-packages/py/py.typed",start:30548,end:30548,audio:0},{filename:"/lib/python3.8/site-packages/py/__init__.pyi",start:30548,end:30889,audio:0},{filename:"/lib/python3.8/site-packages/py/test.py",start:30889,end:31111,audio:0},{filename:"/lib/python3.8/site-packages/py/__init__.py",start:31111,end:37133,audio:0},{filename:"/lib/python3.8/site-packages/py/_version.py",start:37133,end:37275,audio:0},{filename:"/lib/python3.8/site-packages/py/__metainfo.py",start:37275,end:37330,audio:0},{filename:"/lib/python3.8/site-packages/py/error.pyi",start:37330,end:40739,audio:0},{filename:"/lib/python3.8/site-packages/py/_process/forkedfunc.py",start:40739,end:44431,audio:0},{filename:"/lib/python3.8/site-packages/py/_process/killproc.py",start:44431,end:45079,audio:0},{filename:"/lib/python3.8/site-packages/py/_process/cmdexec.py",start:45079,end:46893,audio:0},{filename:"/lib/python3.8/site-packages/py/_process/__init__.py",start:46893,end:46933,audio:0},{filename:"/lib/python3.8/site-packages/py/_code/_assertionold.py",start:46933,end:64802,audio:0},{filename:"/lib/python3.8/site-packages/py/_code/code.py",start:64802,end:92294,audio:0},{filename:"/lib/python3.8/site-packages/py/_code/_py2traceback.py",start:92294,end:95059,audio:0},{filename:"/lib/python3.8/site-packages/py/_code/__init__.py",start:95059,end:95105,audio:0},{filename:"/lib/python3.8/site-packages/py/_code/assertion.py",start:95105,end:98279,audio:0},{filename:"/lib/python3.8/site-packages/py/_code/_assertionnew.py",start:98279,end:109729,audio:0},{filename:"/lib/python3.8/site-packages/py/_code/source.py",start:109729,end:123779,audio:0},{filename:"/lib/python3.8/site-packages/py/_vendored_packages/apipkg.py",start:123779,end:130199,audio:0},{filename:"/lib/python3.8/site-packages/py/_vendored_packages/iniconfig.py",start:130199,end:135407,audio:0},{filename:"/lib/python3.8/site-packages/py/_vendored_packages/__init__.py",start:135407,end:135407,audio:0},{filename:"/lib/python3.8/site-packages/py/_vendored_packages/apipkg-1.4.dist-info/WHEEL",start:135407,end:135517,audio:0},{filename:"/lib/python3.8/site-packages/py/_vendored_packages/apipkg-1.4.dist-info/METADATA",start:135517,end:139008,audio:0},{filename:"/lib/python3.8/site-packages/py/_vendored_packages/apipkg-1.4.dist-info/INSTALLER",start:139008,end:139012,audio:0},{filename:"/lib/python3.8/site-packages/py/_vendored_packages/apipkg-1.4.dist-info/top_level.txt",start:139012,end:139019,audio:0},{filename:"/lib/python3.8/site-packages/py/_vendored_packages/apipkg-1.4.dist-info/metadata.json",start:139019,end:139798,audio:0},{filename:"/lib/python3.8/site-packages/py/_vendored_packages/apipkg-1.4.dist-info/DESCRIPTION.rst",start:139798,end:142601,audio:0},{filename:"/lib/python3.8/site-packages/py/_vendored_packages/apipkg-1.4.dist-info/RECORD",start:142601,end:143265,audio:0},{filename:"/lib/python3.8/site-packages/py/_vendored_packages/iniconfig-1.0.0.dist-info/WHEEL",start:143265,end:143361,audio:0},{filename:"/lib/python3.8/site-packages/py/_vendored_packages/iniconfig-1.0.0.dist-info/METADATA",start:143361,end:145766,audio:0},{filename:"/lib/python3.8/site-packages/py/_vendored_packages/iniconfig-1.0.0.dist-info/INSTALLER",start:145766,end:145770,audio:0},{filename:"/lib/python3.8/site-packages/py/_vendored_packages/iniconfig-1.0.0.dist-info/top_level.txt",start:145770,end:145780,audio:0},{filename:"/lib/python3.8/site-packages/py/_vendored_packages/iniconfig-1.0.0.dist-info/metadata.json",start:145780,end:146730,audio:0},{filename:"/lib/python3.8/site-packages/py/_vendored_packages/iniconfig-1.0.0.dist-info/DESCRIPTION.rst",start:146730,end:148252,audio:0},{filename:"/lib/python3.8/site-packages/py/_vendored_packages/iniconfig-1.0.0.dist-info/RECORD",start:148252,end:148957,audio:0},{filename:"/lib/python3.8/site-packages/py/_path/svnurl.py",start:148957,end:163672,audio:0},{filename:"/lib/python3.8/site-packages/py/_path/common.py",start:163672,end:178490,audio:0},{filename:"/lib/python3.8/site-packages/py/_path/cacheutil.py",start:178490,end:181823,audio:0},{filename:"/lib/python3.8/site-packages/py/_path/local.py",start:181823,end:218582,audio:0},{filename:"/lib/python3.8/site-packages/py/_path/svnwc.py",start:218582,end:262407,audio:0},{filename:"/lib/python3.8/site-packages/py/_path/__init__.py",start:262407,end:262439,audio:0},{filename:"/lib/python3.8/site-packages/py/_log/warning.py",start:262439,end:265004,audio:0},{filename:"/lib/python3.8/site-packages/py/_log/__init__.py",start:265004,end:265078,audio:0},{filename:"/lib/python3.8/site-packages/py/_log/log.py",start:265078,end:271081,audio:0},{filename:"/lib/python3.8/site-packages/py/_io/saferepr.py",start:271081,end:273564,audio:0},{filename:"/lib/python3.8/site-packages/py/_io/terminalwriter.py",start:273564,end:288224,audio:0},{filename:"/lib/python3.8/site-packages/py/_io/__init__.py",start:288224,end:288253,audio:0},{filename:"/lib/python3.8/site-packages/py/_io/capture.py",start:288253,end:299905,audio:0},{filename:"/lib/python3.8/site-packages/py-1.9.0-py3.8.egg-info/SOURCES.txt",start:299905,end:303617,audio:0},{filename:"/lib/python3.8/site-packages/py-1.9.0-py3.8.egg-info/not-zip-safe",start:303617,end:303618,audio:0},{filename:"/lib/python3.8/site-packages/py-1.9.0-py3.8.egg-info/top_level.txt",start:303618,end:303621,audio:0},{filename:"/lib/python3.8/site-packages/py-1.9.0-py3.8.egg-info/PKG-INFO",start:303621,end:306690,audio:0},{filename:"/lib/python3.8/site-packages/py-1.9.0-py3.8.egg-info/dependency_links.txt",start:306690,end:306691,audio:0}],remote_package_size:176024,package_uuid:"00450903-a308-4304-8866-fad0b87389cc"})})();