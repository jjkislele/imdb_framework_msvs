# imdb
## 基于手绘草图的草图检索系统
### 原作者：**Mathias Eitz**

> 在我上一个仓库[imdb_framework](https://github.com/jjkislele/imdb_framework)里有对这个项目的英文说明，不过很迷的是，作者使用的IDE应该是Qt creator，本仓库则将项目代码拆解出来移植至MSVS，方便随后的实验。

![输出示例](./resource/banner.jpg)	

----

#### **说明**

所有代码均fork自[mathiaseitz's imdb_framework](https://github.com/mathiaseitz/imdb_framework)，并在此基础上进行了修改，使之能够符合自己的实验要求。

本项目应出自学术论文["Sketch-Based Shape Retrieval" - SIGGRAPH 2012](http://cybertron.cg.tu-berlin.de/eitz/projects/sbsr/) 与之前的 ["Sketch-based image retrieval: benchmark and bag-of-features descriptors" -  IEEE T VIS COMPUT GR 2011](http://cybertron.cg.tu-berlin.de/eitz/projects/sbsr/)。前者是关于图形轮廓检索的，使用的特征是由作者**Mathias Eitz**等人提出的Gabor滤波改进算法*GALIF*，而后者是关于图像检索的，使用的特征是由作者**Mathias Eitz**等人提出的*HOG*改进算法*SHOG*（当然还有*shape-context*、*shape-context*的改进算法作为对比），并引入**BoF**检索框架（我是这么称呼的），降低了图像特征的复杂度，加快了图像匹配速度。这两篇文章的创新性（经典性），使得后续研究都承接着这两篇文章的思路。

上述两篇文章均是通过手绘草图进行检索，这就不得不提到基于手绘草图的图像检索(**SBIR**, *Sketch-Based Image Retrieval*)这个研究方向，我实际上写了篇关于**SBIR**的综述，日后发到博客上吧。

----

#### **算法**

1. 图像特征

* **GALIF**: *Gabor local line-based feature*

	记得有篇博文对*GALIF*特征进行了详细说明的，很可惜现在找不到了。

* **SHOG**: *Sketched feature lines in the Histogram of Oriented Gradients*
	
	一种改进的*HOG*特征描述子，它在图像边缘处提取*HOG*特征，因此命名为*SHOG*。

2. 检索方式

* **BoF**: Bag of Features

* **Linear search**

-----

#### **项目依赖**

1. boost lib

2. opencv 2.x

3. Qt lib

本项目使用的是boost 1.65.1、opencv 2.4.13、Qt 5，IDE是MSVS 15 x64。各位可以参照``./resource/sbir.props``，在MSVS中配置的时候，也可以导入这个``.props``配置文件，这样就可以方便更改、复用相同配置了。

----

#### **文件夹结构说明**

前文提到，我是将原始项目拆解成了多个模块，此举方便修改原始代码，现将拆解的各模块所对应的文件夹结构说明如下，表格顺序即是程序处理顺序，执行顺序不可发生变化，并且顺带简单介绍模块功能，详细说明则请点击对应链接查看。

序号|文件夹名|功能说明|链接
:--:|--------|--------|----
1|generate_filelist|生成图像绝对路径索引|[:mag:](https://github.com/jjkislele/imdb_framework_msvs/tree/master/imdb/generate_filelist#generate_filelist)
2|compute_descriptors|提取图像特征|[:mag:](https://github.com/jjkislele/imdb_framework_msvs/tree/master/imdb/compute_descriptors#compute_descriptors)
3|compute_vocabulary|特征聚类，产生视觉词汇|[:mag:](https://github.com/jjkislele/imdb_framework_msvs/tree/master/imdb/compute_vocabulary#compute_vocabulary)
4|compute_histvw|由视觉词汇产生视觉词汇表|[:mag:](https://github.com/jjkislele/imdb_framework_msvs/tree/master/imdb/compute_histvw#compute_histvw)
5|compute_index|生成TF-IDF权重|[:mag:](https://github.com/jjkislele/imdb_framework_msvs/tree/master/imdb/compute_index#compute_index)
6|image_search|图像检索|[:mag:](https://github.com/jjkislele/imdb_framework_msvs/tree/master/imdb/image_search#image_search)
7|image_search_featureExtracted|image_search的修改版本，输入为检索序列特征|[:mag:](https://github.com/jjkislele/imdb_framework_msvs/tree/master/imdb/image_search_featureExtracted#image_search_featureextracted)