var Module=typeof pyodide._module!=="undefined"?pyodide._module:{};if(!Module.expectedDataFileDownloads){Module.expectedDataFileDownloads=0;Module.finishedDataFileDownloads=0}Module.expectedDataFileDownloads++;(function(){var loadPackage=function(metadata){var PACKAGE_PATH;if(typeof window==="object"){PACKAGE_PATH=window["encodeURIComponent"](window.location.pathname.toString().substring(0,window.location.pathname.toString().lastIndexOf("/"))+"/")}else if(typeof location!=="undefined"){PACKAGE_PATH=encodeURIComponent(location.pathname.toString().substring(0,location.pathname.toString().lastIndexOf("/"))+"/")}else{throw"using preloaded data can only be done on a web page or in a web worker"}var PACKAGE_NAME="regex.data";var REMOTE_PACKAGE_BASE="regex.data";if(typeof Module["locateFilePackage"]==="function"&&!Module["locateFile"]){Module["locateFile"]=Module["locateFilePackage"];err("warning: you defined Module.locateFilePackage, that has been renamed to Module.locateFile (using your locateFilePackage for now)")}var REMOTE_PACKAGE_NAME=Module["locateFile"]?Module["locateFile"](REMOTE_PACKAGE_BASE,""):REMOTE_PACKAGE_BASE;var REMOTE_PACKAGE_SIZE=metadata.remote_package_size;var PACKAGE_UUID=metadata.package_uuid;function fetchRemotePackage(packageName,packageSize,callback,errback){var xhr=new XMLHttpRequest;xhr.open("GET",packageName,true);xhr.responseType="arraybuffer";xhr.onprogress=function(event){var url=packageName;var size=packageSize;if(event.total)size=event.total;if(event.loaded){if(!xhr.addedTotal){xhr.addedTotal=true;if(!Module.dataFileDownloads)Module.dataFileDownloads={};Module.dataFileDownloads[url]={loaded:event.loaded,total:size}}else{Module.dataFileDownloads[url].loaded=event.loaded}var total=0;var loaded=0;var num=0;for(var download in Module.dataFileDownloads){var data=Module.dataFileDownloads[download];total+=data.total;loaded+=data.loaded;num++}total=Math.ceil(total*Module.expectedDataFileDownloads/num);if(Module["setStatus"])Module["setStatus"]("Downloading data... ("+loaded+"/"+total+")")}else if(!Module.dataFileDownloads){if(Module["setStatus"])Module["setStatus"]("Downloading data...")}};xhr.onerror=function(event){throw new Error("NetworkError for: "+packageName)};xhr.onload=function(event){if(xhr.status==200||xhr.status==304||xhr.status==206||xhr.status==0&&xhr.response){var packageData=xhr.response;callback(packageData)}else{throw new Error(xhr.statusText+" : "+xhr.responseURL)}};xhr.send(null)}function handleError(error){console.error("package error:",error)}var fetchedCallback=null;var fetched=Module["getPreloadedPackage"]?Module["getPreloadedPackage"](REMOTE_PACKAGE_NAME,REMOTE_PACKAGE_SIZE):null;if(!fetched)fetchRemotePackage(REMOTE_PACKAGE_NAME,REMOTE_PACKAGE_SIZE,function(data){if(fetchedCallback){fetchedCallback(data);fetchedCallback=null}else{fetched=data}},handleError);function runWithFS(){function assert(check,msg){if(!check)throw msg+(new Error).stack}Module["FS_createPath"]("/","lib",true,true);Module["FS_createPath"]("/lib","python3.8",true,true);Module["FS_createPath"]("/lib/python3.8","site-packages",true,true);Module["FS_createPath"]("/lib/python3.8/site-packages","regex",true,true);Module["FS_createPath"]("/lib/python3.8/site-packages","regex-2020.7.14-py3.8.egg-info",true,true);function DataRequest(start,end,audio){this.start=start;this.end=end;this.audio=audio}DataRequest.prototype={requests:{},open:function(mode,name){this.name=name;this.requests[name]=this;Module["addRunDependency"]("fp "+this.name)},send:function(){},onload:function(){var byteArray=this.byteArray.subarray(this.start,this.end);this.finish(byteArray)},finish:function(byteArray){var that=this;Module["FS_createPreloadedFile"](this.name,null,byteArray,true,true,function(){Module["removeRunDependency"]("fp "+that.name)},function(){if(that.audio){Module["removeRunDependency"]("fp "+that.name)}else{err("Preloading file "+that.name+" failed")}},false,true);this.requests[this.name]=null}};function processPackageData(arrayBuffer){Module.finishedDataFileDownloads++;assert(arrayBuffer,"Loading data file failed.");assert(arrayBuffer instanceof ArrayBuffer,"bad input to processPackageData");var byteArray=new Uint8Array(arrayBuffer);var curr;var compressedData={data:null,cachedOffset:540269,cachedIndexes:[-1,-1],cachedChunks:[null,null],offsets:[0,1279,2476,3842,5062,6238,7451,8615,9694,11039,12260,13443,14458,15519,16886,18256,19684,21194,22624,23832,25024,26435,27848,28934,30342,31312,32575,33603,34254,34743,35732,36603,37706,38453,39409,40081,41157,42172,43240,44559,45629,46862,47934,48777,49953,51362,52400,53525,54751,55801,56475,57052,57720,58472,59035,59638,60770,61515,62377,63197,63496,64505,65642,66574,67836,69023,69553,70610,71633,72678,73828,74695,75764,76745,77769,79039,80117,81224,82158,83246,84022,84956,85956,87011,88207,88499,88807,89089,89261,89452,90131,90332,90626,90861,91165,91450,91633,91667,92649,93688,94235,94644,94987,95592,96308,97081,98018,98929,100175,100914,102184,103283,104213,105211,106325,107494,108537,109504,110136,110515,111780,112673,113850,115035,116304,116689,117062,117640,118454,119317,120174,121013,121687,122540,123210,123882,124306,124947,125500,126111,126886,127573,128450,128795,129193,129992,131220,132466,133451,134259,135605,136827,138104,139457,140885,141771,143141,144455,145107,146390,147768,149137,150281,151712,153058,153907,155150,156080,157214,158408,159599,160717,162004,163371,164726,165978,166469,166494,166519,166970,167174,167206,167237,167262,167881,168267,168584,169543,170883,172377,173536,174526,175608,176663,177707,178759,179815,180873,181925,182981,184040,185088,186145,187203,188265,189316,190376,191436,192495,193547,194608,195669,196715,197160,197836,198511,199249,199946,200871,201552,201577,201602,202651,204561,206352,208277,210325,211485,212374,213441,214532,215457,216126,216793,217779,218548,219277,219987,220698,221449,222120,223031,223754,224633,225213,225494,226095,226336,226922,228167,228741,229500,230331,231118,231849,232571,233325,233988,234758,235723,236216,236969,237548,238006,238556,239098,239633,240207,240605,241221,242075,243311,243976,244905,246147,246914,247347,247555,247913,248559,249168,249936,250625,251401,252112,252669,252968,253525,254347,255096,255749,256220,256827,257395,257831,258525,259340,260294,261120,261936,262772,263555,264229,265058,265747,266555,267131,267849,268539,269149,269611,270341,271222,272014,272501,273360,274077,275312,276098,277199,277987,278761,278954,279327,280216,280827,281487,282161,282752,283275,284376,285255,286680,287580,288753,289652,290689,291372,292318,293095,293880,294670,295410,296250,296921,297636,297940,298345,298826,298975,299547,300371,301016,301426,302238,302980,303638,304291,304892,305116,305748,306348,307172,308232,308869,309513,310237,310815,311451,312187,312532,312717,312904,313085,313293,313473,313660,313844,314026,314219,314405,314591,314729,315462,316369,317014,317739,318353,318983,319410,320505,321553,322849,324365,325270,325834,326641,327202,327706,328281,328797,329308,329864,330619,331167,332067,332912,333863,334829,335694,336697,337283,338593,340047,341445,342718,344155,345507,346974,348456,350099,351071,352227,353484,354545,355881,357260,358088,358918,359877,360979,362251,363371,364677,366039,367270,368539,369388,370142,370764,371482,372301,372891,373674,374416,375293,375764,376497,377153,377779,378291,378856,379614,380460,381604,382460,383291,383897,384725,385265,385545,386169,386681,387476,388056,388673,389286,389886,390624,391212,391894,392788,393437,393968,394497,395085,395835,396698,397556,398151,398813,399638,400417,401132,401762,402563,403392,403904,404571,405314,406143,407184,408276,409295,410306,411301,412189,412920,413596,414196,415160,415721,416471,417097,418032,418732,419506,420146,420652,421361,422237,422966,423839,424763,426072,426835,427466,428536,429022,429577,430429,431404,432093,432616,433763,434583,435489,436571,437478,438051,438706,439424,439989,440279,441570,442770,443480,443853,444177,445013,446474,447887,449253,450673,451886,452914,453582,454576,455649,456503,457522,458554,459536,460553,461377,462491,463590,464628,465614,466624,467768,468608,469661,470878,471850,472991,474194,475305,476195,477206,478219,479304,480315,481206,482170,483311,484401,485541,486461,487347,488234,489274,490308,491199,492173,493172,494102,495098,496170,497325,498363,499446,500402,501401,502524,503434,504299,505157,506168,507209,508372,509152,509832,510675,511767,512872,514108,515383,516552,517596,518719,519965,521048,521978,522952,524139,524976,525826,526548,527535,528687,529668,530789,531748,532910,534132,535039,536048,537091,538029,539088,540221],sizes:[1279,1197,1366,1220,1176,1213,1164,1079,1345,1221,1183,1015,1061,1367,1370,1428,1510,1430,1208,1192,1411,1413,1086,1408,970,1263,1028,651,489,989,871,1103,747,956,672,1076,1015,1068,1319,1070,1233,1072,843,1176,1409,1038,1125,1226,1050,674,577,668,752,563,603,1132,745,862,820,299,1009,1137,932,1262,1187,530,1057,1023,1045,1150,867,1069,981,1024,1270,1078,1107,934,1088,776,934,1e3,1055,1196,292,308,282,172,191,679,201,294,235,304,285,183,34,982,1039,547,409,343,605,716,773,937,911,1246,739,1270,1099,930,998,1114,1169,1043,967,632,379,1265,893,1177,1185,1269,385,373,578,814,863,857,839,674,853,670,672,424,641,553,611,775,687,877,345,398,799,1228,1246,985,808,1346,1222,1277,1353,1428,886,1370,1314,652,1283,1378,1369,1144,1431,1346,849,1243,930,1134,1194,1191,1118,1287,1367,1355,1252,491,25,25,451,204,32,31,25,619,386,317,959,1340,1494,1159,990,1082,1055,1044,1052,1056,1058,1052,1056,1059,1048,1057,1058,1062,1051,1060,1060,1059,1052,1061,1061,1046,445,676,675,738,697,925,681,25,25,1049,1910,1791,1925,2048,1160,889,1067,1091,925,669,667,986,769,729,710,711,751,671,911,723,879,580,281,601,241,586,1245,574,759,831,787,731,722,754,663,770,965,493,753,579,458,550,542,535,574,398,616,854,1236,665,929,1242,767,433,208,358,646,609,768,689,776,711,557,299,557,822,749,653,471,607,568,436,694,815,954,826,816,836,783,674,829,689,808,576,718,690,610,462,730,881,792,487,859,717,1235,786,1101,788,774,193,373,889,611,660,674,591,523,1101,879,1425,900,1173,899,1037,683,946,777,785,790,740,840,671,715,304,405,481,149,572,824,645,410,812,742,658,653,601,224,632,600,824,1060,637,644,724,578,636,736,345,185,187,181,208,180,187,184,182,193,186,186,138,733,907,645,725,614,630,427,1095,1048,1296,1516,905,564,807,561,504,575,516,511,556,755,548,900,845,951,966,865,1003,586,1310,1454,1398,1273,1437,1352,1467,1482,1643,972,1156,1257,1061,1336,1379,828,830,959,1102,1272,1120,1306,1362,1231,1269,849,754,622,718,819,590,783,742,877,471,733,656,626,512,565,758,846,1144,856,831,606,828,540,280,624,512,795,580,617,613,600,738,588,682,894,649,531,529,588,750,863,858,595,662,825,779,715,630,801,829,512,667,743,829,1041,1092,1019,1011,995,888,731,676,600,964,561,750,626,935,700,774,640,506,709,876,729,873,924,1309,763,631,1070,486,555,852,975,689,523,1147,820,906,1082,907,573,655,718,565,290,1291,1200,710,373,324,836,1461,1413,1366,1420,1213,1028,668,994,1073,854,1019,1032,982,1017,824,1114,1099,1038,986,1010,1144,840,1053,1217,972,1141,1203,1111,890,1011,1013,1085,1011,891,964,1141,1090,1140,920,886,887,1040,1034,891,974,999,930,996,1072,1155,1038,1083,956,999,1123,910,865,858,1011,1041,1163,780,680,843,1092,1105,1236,1275,1169,1044,1123,1246,1083,930,974,1187,837,850,722,987,1152,981,1121,959,1162,1222,907,1009,1043,938,1059,1133,48],successes:[1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0]};compressedData.data=byteArray;assert(typeof Module.LZ4==="object","LZ4 not present - was your app build with  -s LZ4=1  ?");Module.LZ4.loadPackage({metadata:metadata,compressedData:compressedData});Module["removeRunDependency"]("datafile_regex.data")}Module["addRunDependency"]("datafile_regex.data");if(!Module.preloadResults)Module.preloadResults={};Module.preloadResults[PACKAGE_NAME]={fromCache:false};if(fetched){processPackageData(fetched);fetched=null}else{fetchedCallback=processPackageData}}if(Module["calledRun"]){runWithFS()}else{if(!Module["preRun"])Module["preRun"]=[];Module["preRun"].push(runWithFS)}};loadPackage({files:[{filename:"/lib/python3.8/site-packages/regex/_regex.so",start:0,end:858868,audio:0},{filename:"/lib/python3.8/site-packages/regex/regex.py",start:858868,end:890775,audio:0},{filename:"/lib/python3.8/site-packages/regex/test_regex.py",start:890775,end:1103957,audio:0},{filename:"/lib/python3.8/site-packages/regex/__init__.py",start:1103957,end:1104022,audio:0},{filename:"/lib/python3.8/site-packages/regex/_regex_core.py",start:1104022,end:1244177,audio:0},{filename:"/lib/python3.8/site-packages/regex-2020.7.14-py3.8.egg-info/SOURCES.txt",start:1244177,end:1244759,audio:0},{filename:"/lib/python3.8/site-packages/regex-2020.7.14-py3.8.egg-info/top_level.txt",start:1244759,end:1244765,audio:0},{filename:"/lib/python3.8/site-packages/regex-2020.7.14-py3.8.egg-info/PKG-INFO",start:1244765,end:1292335,audio:0},{filename:"/lib/python3.8/site-packages/regex-2020.7.14-py3.8.egg-info/dependency_links.txt",start:1292335,end:1292336,audio:0}],remote_package_size:544365,package_uuid:"bfe4a27d-477b-4e05-ad5a-05c5de268ffc"})})();