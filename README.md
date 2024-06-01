Как запустить проект:<br>
1. Необходимо иметь установленным любой компилятор С++ версии 17 и выше<br>
2. Обработать грамматику языка в файле MyGrammar.atg по средствам CoCo/R:<br>
Coco MyGrammar.atg<br>
3. Скомпилировать полученные файлы (Scanner.cpp + Scanner.h, Parser.cpp + Parser.h) и main.cpp в исполняемый файл:<br>
g++ -fno-exceptions -std=c++23 -O3 -o program main.cpp Scanner.cpp Parser.cpp (build.bat)<br>
В данном примере используется компилятор g++ версии 13.2.0<br>
4. Для запуска нужно запустить программу интерпретатора с именем файла программы на языке интерпретатора<br>
program Test.txt (run.bat)<br>
5. Также можно добавить флаг t после имени файла для отображения времени работы интерпретатора<br>
program Test.txt t (runt.bat)
