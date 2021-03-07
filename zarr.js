var Module=typeof pyodide._module!=="undefined"?pyodide._module:{};if(!Module.expectedDataFileDownloads){Module.expectedDataFileDownloads=0;Module.finishedDataFileDownloads=0}Module.expectedDataFileDownloads++;(function(){var loadPackage=function(metadata){var PACKAGE_PATH;if(typeof window==="object"){PACKAGE_PATH=window["encodeURIComponent"](window.location.pathname.toString().substring(0,window.location.pathname.toString().lastIndexOf("/"))+"/")}else if(typeof location!=="undefined"){PACKAGE_PATH=encodeURIComponent(location.pathname.toString().substring(0,location.pathname.toString().lastIndexOf("/"))+"/")}else{throw"using preloaded data can only be done on a web page or in a web worker"}var PACKAGE_NAME="zarr.data";var REMOTE_PACKAGE_BASE="zarr.data";if(typeof Module["locateFilePackage"]==="function"&&!Module["locateFile"]){Module["locateFile"]=Module["locateFilePackage"];err("warning: you defined Module.locateFilePackage, that has been renamed to Module.locateFile (using your locateFilePackage for now)")}var REMOTE_PACKAGE_NAME=Module["locateFile"]?Module["locateFile"](REMOTE_PACKAGE_BASE,""):REMOTE_PACKAGE_BASE;var REMOTE_PACKAGE_SIZE=metadata.remote_package_size;var PACKAGE_UUID=metadata.package_uuid;function fetchRemotePackage(packageName,packageSize,callback,errback){var xhr=new XMLHttpRequest;xhr.open("GET",packageName,true);xhr.responseType="arraybuffer";xhr.onprogress=function(event){var url=packageName;var size=packageSize;if(event.total)size=event.total;if(event.loaded){if(!xhr.addedTotal){xhr.addedTotal=true;if(!Module.dataFileDownloads)Module.dataFileDownloads={};Module.dataFileDownloads[url]={loaded:event.loaded,total:size}}else{Module.dataFileDownloads[url].loaded=event.loaded}var total=0;var loaded=0;var num=0;for(var download in Module.dataFileDownloads){var data=Module.dataFileDownloads[download];total+=data.total;loaded+=data.loaded;num++}total=Math.ceil(total*Module.expectedDataFileDownloads/num);if(Module["setStatus"])Module["setStatus"]("Downloading data... ("+loaded+"/"+total+")")}else if(!Module.dataFileDownloads){if(Module["setStatus"])Module["setStatus"]("Downloading data...")}};xhr.onerror=function(event){throw new Error("NetworkError for: "+packageName)};xhr.onload=function(event){if(xhr.status==200||xhr.status==304||xhr.status==206||xhr.status==0&&xhr.response){var packageData=xhr.response;callback(packageData)}else{throw new Error(xhr.statusText+" : "+xhr.responseURL)}};xhr.send(null)}function handleError(error){console.error("package error:",error)}var fetchedCallback=null;var fetched=Module["getPreloadedPackage"]?Module["getPreloadedPackage"](REMOTE_PACKAGE_NAME,REMOTE_PACKAGE_SIZE):null;if(!fetched)fetchRemotePackage(REMOTE_PACKAGE_NAME,REMOTE_PACKAGE_SIZE,function(data){if(fetchedCallback){fetchedCallback(data);fetchedCallback=null}else{fetched=data}},handleError);function runWithFS(){function assert(check,msg){if(!check)throw msg+(new Error).stack}Module["FS_createPath"]("/","lib",true,true);Module["FS_createPath"]("/lib","python3.8",true,true);Module["FS_createPath"]("/lib/python3.8","site-packages",true,true);Module["FS_createPath"]("/lib/python3.8/site-packages","zarr-2.6.1-py3.8.egg-info",true,true);Module["FS_createPath"]("/lib/python3.8/site-packages","zarr",true,true);Module["FS_createPath"]("/lib/python3.8/site-packages/zarr","tests",true,true);function DataRequest(start,end,audio){this.start=start;this.end=end;this.audio=audio}DataRequest.prototype={requests:{},open:function(mode,name){this.name=name;this.requests[name]=this;Module["addRunDependency"]("fp "+this.name)},send:function(){},onload:function(){var byteArray=this.byteArray.subarray(this.start,this.end);this.finish(byteArray)},finish:function(byteArray){var that=this;Module["FS_createPreloadedFile"](this.name,null,byteArray,true,true,function(){Module["removeRunDependency"]("fp "+that.name)},function(){if(that.audio){Module["removeRunDependency"]("fp "+that.name)}else{err("Preloading file "+that.name+" failed")}},false,true);this.requests[this.name]=null}};function processPackageData(arrayBuffer){Module.finishedDataFileDownloads++;assert(arrayBuffer,"Loading data file failed.");assert(arrayBuffer instanceof ArrayBuffer,"bad input to processPackageData");var byteArray=new Uint8Array(arrayBuffer);var curr;var compressedData={data:null,cachedOffset:367016,cachedIndexes:[-1,-1],cachedChunks:[null,null],offsets:[0,1061,1733,2376,2959,3565,4153,4771,5359,5936,6548,7137,7721,8331,8914,9502,10114,10708,11306,11899,12485,13072,13674,14269,14853,15445,15961,16511,17016,17559,18100,18755,19357,19948,20562,21148,21740,22324,22896,23496,24088,24673,25248,25843,26434,27028,27598,28193,28780,29364,29934,30531,31118,31703,32273,32904,33541,34155,34786,35466,36105,36716,37381,37998,38808,39756,40788,42104,43489,44418,45565,46834,47733,48832,50132,51426,52258,53181,54507,55844,57020,58153,59293,60416,61442,62697,63811,65055,66294,67672,68837,69763,70984,72214,73595,74618,75920,77191,77989,79322,80291,81266,82406,83360,84176,85473,86497,87646,88761,90005,91259,92791,94044,95326,96528,97765,98876,100076,101389,102483,103614,104852,106e3,107081,108202,109216,110288,111183,112244,113148,114199,115213,116441,117566,118705,119293,119828,120540,121764,122972,123965,125106,126142,127483,128235,129171,130170,131191,132120,133077,133836,134857,135937,137219,138339,139526,140659,141456,142792,144077,145258,146116,147341,148266,149293,150311,151137,152397,153705,154978,156222,157522,158572,159812,160812,162128,163015,164088,165220,166476,167894,169262,170441,171533,172758,174084,175255,176236,177295,178254,179367,180574,181415,182678,183499,184624,185851,186707,187912,189112,190383,191598,192647,193623,194796,195627,196751,197927,199092,200319,201094,202158,203169,204244,205320,206543,207630,208475,209629,210835,212023,212966,214069,215453,216655,217750,218863,220018,221114,222171,223357,224486,225514,226556,227659,228708,229594,230723,231842,232563,233546,234316,234882,235621,236441,237336,238120,238971,239780,240833,242017,242947,243833,244709,245956,246949,247751,248765,249748,250649,251533,252294,252927,253657,254812,255793,256651,257699,258683,259920,261015,262027,263070,263874,264852,265866,266971,267787,268603,269421,270364,271184,271837,272837,273788,274468,275331,276277,277223,278191,279178,280041,281092,282035,283038,284015,284953,286036,286939,287955,289019,289463,290419,291218,292190,293013,293769,294736,295449,296111,296885,297724,298502,298974,299798,300646,301435,301955,303041,303935,304990,305789,306630,307686,308583,309593,310611,311430,312356,313503,314629,315513,316058,316816,317763,318430,319575,320438,321256,322027,323074,323673,324635,325551,326652,327635,328614,329487,330346,331230,331971,332872,333700,334495,335222,335868,336725,337661,338440,339430,340319,341133,341875,342547,343204,344044,345003,345937,346930,347890,348794,349538,350458,351323,352207,353097,354189,354843,355799,356724,357694,358727,359587,360439,361179,362276,363286,364372,365392,366367],sizes:[1061,672,643,583,606,588,618,588,577,612,589,584,610,583,588,612,594,598,593,586,587,602,595,584,592,516,550,505,543,541,655,602,591,614,586,592,584,572,600,592,585,575,595,591,594,570,595,587,584,570,597,587,585,570,631,637,614,631,680,639,611,665,617,810,948,1032,1316,1385,929,1147,1269,899,1099,1300,1294,832,923,1326,1337,1176,1133,1140,1123,1026,1255,1114,1244,1239,1378,1165,926,1221,1230,1381,1023,1302,1271,798,1333,969,975,1140,954,816,1297,1024,1149,1115,1244,1254,1532,1253,1282,1202,1237,1111,1200,1313,1094,1131,1238,1148,1081,1121,1014,1072,895,1061,904,1051,1014,1228,1125,1139,588,535,712,1224,1208,993,1141,1036,1341,752,936,999,1021,929,957,759,1021,1080,1282,1120,1187,1133,797,1336,1285,1181,858,1225,925,1027,1018,826,1260,1308,1273,1244,1300,1050,1240,1e3,1316,887,1073,1132,1256,1418,1368,1179,1092,1225,1326,1171,981,1059,959,1113,1207,841,1263,821,1125,1227,856,1205,1200,1271,1215,1049,976,1173,831,1124,1176,1165,1227,775,1064,1011,1075,1076,1223,1087,845,1154,1206,1188,943,1103,1384,1202,1095,1113,1155,1096,1057,1186,1129,1028,1042,1103,1049,886,1129,1119,721,983,770,566,739,820,895,784,851,809,1053,1184,930,886,876,1247,993,802,1014,983,901,884,761,633,730,1155,981,858,1048,984,1237,1095,1012,1043,804,978,1014,1105,816,816,818,943,820,653,1e3,951,680,863,946,946,968,987,863,1051,943,1003,977,938,1083,903,1016,1064,444,956,799,972,823,756,967,713,662,774,839,778,472,824,848,789,520,1086,894,1055,799,841,1056,897,1010,1018,819,926,1147,1126,884,545,758,947,667,1145,863,818,771,1047,599,962,916,1101,983,979,873,859,884,741,901,828,795,727,646,857,936,779,990,889,814,742,672,657,840,959,934,993,960,904,744,920,865,884,890,1092,654,956,925,970,1033,860,852,740,1097,1010,1086,1020,975,649],successes:[1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]};compressedData.data=byteArray;assert(typeof Module.LZ4==="object","LZ4 not present - was your app build with  -s LZ4=1  ?");Module.LZ4.loadPackage({metadata:metadata,compressedData:compressedData});Module["removeRunDependency"]("datafile_zarr.data")}Module["addRunDependency"]("datafile_zarr.data");if(!Module.preloadResults)Module.preloadResults={};Module.preloadResults[PACKAGE_NAME]={fromCache:false};if(fetched){processPackageData(fetched);fetched=null}else{fetchedCallback=processPackageData}}if(Module["calledRun"]){runWithFS()}else{if(!Module["preRun"])Module["preRun"]=[];Module["preRun"].push(runWithFS)}};loadPackage({files:[{filename:"/lib/python3.8/site-packages/zarr-2.6.1-py3.8.egg-info/SOURCES.txt",start:0,end:131132,audio:0},{filename:"/lib/python3.8/site-packages/zarr-2.6.1-py3.8.egg-info/top_level.txt",start:131132,end:131137,audio:0},{filename:"/lib/python3.8/site-packages/zarr-2.6.1-py3.8.egg-info/PKG-INFO",start:131137,end:136440,audio:0},{filename:"/lib/python3.8/site-packages/zarr-2.6.1-py3.8.egg-info/requires.txt",start:136440,end:136506,audio:0},{filename:"/lib/python3.8/site-packages/zarr-2.6.1-py3.8.egg-info/dependency_links.txt",start:136506,end:136507,audio:0},{filename:"/lib/python3.8/site-packages/zarr/storage.py",start:136507,end:225148,audio:0},{filename:"/lib/python3.8/site-packages/zarr/util.py",start:225148,end:240840,audio:0},{filename:"/lib/python3.8/site-packages/zarr/meta.py",start:240840,end:246345,audio:0},{filename:"/lib/python3.8/site-packages/zarr/hierarchy.py",start:246345,end:286474,audio:0},{filename:"/lib/python3.8/site-packages/zarr/attrs.py",start:286474,end:290343,audio:0},{filename:"/lib/python3.8/site-packages/zarr/codecs.py",start:290343,end:290493,audio:0},{filename:"/lib/python3.8/site-packages/zarr/n5.py",start:290493,end:311066,audio:0},{filename:"/lib/python3.8/site-packages/zarr/creation.py",start:311066,end:330150,audio:0},{filename:"/lib/python3.8/site-packages/zarr/convenience.py",start:330150,end:371248,audio:0},{filename:"/lib/python3.8/site-packages/zarr/meta_v1.py",start:371248,end:372903,audio:0},{filename:"/lib/python3.8/site-packages/zarr/errors.py",start:372903,end:374706,audio:0},{filename:"/lib/python3.8/site-packages/zarr/sync.py",start:374706,end:375897,audio:0},{filename:"/lib/python3.8/site-packages/zarr/__init__.py",start:375897,end:376984,audio:0},{filename:"/lib/python3.8/site-packages/zarr/core.py",start:376984,end:456920,audio:0},{filename:"/lib/python3.8/site-packages/zarr/indexing.py",start:456920,end:484910,audio:0},{filename:"/lib/python3.8/site-packages/zarr/version.py",start:484910,end:485052,audio:0},{filename:"/lib/python3.8/site-packages/zarr/tests/test_storage.py",start:485052,end:548289,audio:0},{filename:"/lib/python3.8/site-packages/zarr/tests/util.py",start:548289,end:549658,audio:0},{filename:"/lib/python3.8/site-packages/zarr/tests/test_convenience.py",start:549658,end:573929,audio:0},{filename:"/lib/python3.8/site-packages/zarr/tests/test_util.py",start:573929,end:580211,audio:0},{filename:"/lib/python3.8/site-packages/zarr/tests/test_indexing.py",start:580211,end:620151,audio:0},{filename:"/lib/python3.8/site-packages/zarr/tests/test_hierarchy.py",start:620151,end:660457,audio:0},{filename:"/lib/python3.8/site-packages/zarr/tests/test_meta.py",start:660457,end:673196,audio:0},{filename:"/lib/python3.8/site-packages/zarr/tests/test_info.py",start:673196,end:674195,audio:0},{filename:"/lib/python3.8/site-packages/zarr/tests/test_attrs.py",start:674195,end:681659,audio:0},{filename:"/lib/python3.8/site-packages/zarr/tests/__init__.py",start:681659,end:681659,audio:0},{filename:"/lib/python3.8/site-packages/zarr/tests/test_filters.py",start:681659,end:687555,audio:0},{filename:"/lib/python3.8/site-packages/zarr/tests/test_creation.py",start:687555,end:702352,audio:0},{filename:"/lib/python3.8/site-packages/zarr/tests/test_core.py",start:702352,end:789089,audio:0},{filename:"/lib/python3.8/site-packages/zarr/tests/test_sync.py",start:789089,end:798402,audio:0}],remote_package_size:371112,package_uuid:"f3b3df8a-d6af-4d5a-bc03-55acea969fb1"})})();