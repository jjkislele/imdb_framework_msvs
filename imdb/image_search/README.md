#### image_search

----------------

**说明**

``image_search_featureExtracted``是本程序``image_search``的改进版本，除一下点外，两者并无其他差异，因此可阅读``image_search_featureExtracted``的说明。

**差异**

``image_search``本程序在``main.cpp``主函数的第217行，使用函数``gen->compute(data);``，也即是，读取每张检索序列的图像路径后（具体来说是``main.cpp``主函数的第212至213行，``queryFiles.get_filename()``与``cv::imread()``），调用``generator.cpp``中的函数计算每张图像的特征，因此计算速度较慢、耗费的内存也较多，因此就有了改进版本``image_search_featureExtracted``。

``image_search_featureExtracted``使用``-d``选项，直接输入已经处理好的检索序列特征集合。