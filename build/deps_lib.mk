src/ecss/user_agent.o: src/ecss/user_agent.cpp src/ecss/user_agent.h
src/ecss/tokenize.o: src/ecss/tokenize.cpp src/ecss/lexer.h src/ecss/tokens.h \
  src/ecss/utility.h src/ecss/file_util.h
src/ecss/evaluator.o: src/ecss/evaluator.cpp src/ecss/evaluator.h src/ecss/ast.h \
  src/ecss/lexer.h src/ecss/tokens.h src/ecss/utility.h \
  src/ecss/user_agent.h src/ecss/error.h src/ecss/symbol_table.h
src/ecss/lexer.o: src/ecss/lexer.cpp src/ecss/lexer.h src/ecss/tokens.h \
  src/ecss/utility.h src/ecss/scanner_aux.h src/ecss/scanner.l.h
src/ecss/file_util.o: src/ecss/file_util.cpp src/ecss/file_util.h src/ecss/error.h \
  src/ecss/lexer.h src/ecss/tokens.h src/ecss/utility.h
src/ecss/symbol_table.o: src/ecss/symbol_table.cpp src/ecss/symbol_table.h \
  src/ecss/lexer.h src/ecss/tokens.h src/ecss/utility.h
src/ecss/settings.o: src/ecss/settings.cpp src/ecss/settings.h src/ecss/regex.h \
  src/ecss/utility.h src/ecss/user_agent.h
src/ecss/error.o: src/ecss/error.cpp src/ecss/error.h src/ecss/lexer.h \
  src/ecss/tokens.h src/ecss/utility.h
src/ecss/parser.o: src/ecss/parser.cpp src/ecss/parser.h src/ecss/ast.h \
  src/ecss/lexer.h src/ecss/tokens.h src/ecss/utility.h \
  src/ecss/user_agent.h src/ecss/error.h src/ecss/settings.h \
  src/ecss/regex.h src/ecss/file_util.h
src/ecss/ast.o: src/ecss/ast.cpp src/ecss/ast.h src/ecss/lexer.h src/ecss/tokens.h \
  src/ecss/utility.h src/ecss/user_agent.h
src/ecss/css.o: src/ecss/css.cpp src/ecss/css.h src/ecss/utility.h
src/ecss/translator.o: src/ecss/translator.cpp src/ecss/translator.h \
  src/ecss/ast.h src/ecss/lexer.h src/ecss/tokens.h src/ecss/utility.h \
  src/ecss/user_agent.h src/ecss/css.h src/ecss/error.h \
  src/ecss/settings.h src/ecss/regex.h src/ecss/symbol_table.h \
  src/ecss/evaluator.h src/ecss/optimizer.h
src/ecss/regex.o: src/ecss/regex.cpp src/ecss/regex.h src/ecss/utility.h \
  src/ecss/error.h src/ecss/lexer.h src/ecss/tokens.h
src/ecss/utility.o: src/ecss/utility.cpp src/ecss/utility.h
src/ecss/ecss.o: src/ecss/ecss.cpp src/ecss/file_util.h src/ecss/parser.h \
  src/ecss/ast.h src/ecss/lexer.h src/ecss/tokens.h src/ecss/utility.h \
  src/ecss/user_agent.h src/ecss/error.h src/ecss/settings.h \
  src/ecss/regex.h src/ecss/translator.h src/ecss/css.h \
  src/ecss/symbol_table.h
src/ecss/tokens.o: src/ecss/tokens.cpp src/ecss/tokens.h src/ecss/utility.h
src/ecss/optimizer.o: src/ecss/optimizer.cpp src/ecss/optimizer.h src/ecss/css.h \
  src/ecss/settings.h src/ecss/regex.h src/ecss/utility.h \
  src/ecss/user_agent.h
