python3 build_analyzer.py
install_name_tool -change @executable_path/libAnalyzer.dylib @rpath/libAnalyzer.dylib ./debug/libVTSAnalyzer.dylib
