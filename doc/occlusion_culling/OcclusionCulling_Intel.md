vtk可能和遮挡剔除相关的类：vtkVisibilityCuller https://adamdjellouli.com/articles/vtk_examples/07_performance_optimization_and_parallelism

OpenGL中OcclusionCulling类：https://visualizationlibrary.org/documentation/classvl_1_1_occlusion_cull_renderer.html#_details

### README

The technique used in this sample divides scene objects into occluders and occludees and culls occludees based on a depth comparison with the occluders that are software rasterized to the depth buffer. The sample code uses frustum culling and is optimized with Streaming SIMD Extensions (SSE) and Advanced Vector Extensions (AVX) instruction sets and multi-threading to achieve up to 8X performance speedup compared to a non-culled display of the sample scene.

本示例中使用的技术将场景对象划分为遮挡物和遮挡物，并基于与软件光栅化到深度缓冲区的遮挡物的深度比较来剔除遮挡物（？）。示例代码使用截锥体剔除（？），并使用流式SIMD扩展(SSE)和高级矢量扩展(AVX)指令集和多线程进行优化，与示例场景的非筛选显示相比，可实现高达8倍的性能加速。

For a detailed explanation, please see this [article](https://software.intel.com/en-us/articles/software-occlusion-culling).

In addition to the technique detailed above, a separate Masked Occlusion Culling library has been included for comparison purposes, with its main up-to-date codebase being developed at https://github.com/GameTechDev/MaskedOcclusionCulling

**As of late 2017 the Software Occlusion Culling Sample code is in the maintenance mode; reported bugs will be fixed but no upgrades are planned so far. However, Masked Occlusion Culling technique is still being actively developed.**

## 技术文章

链接：https://www.intel.com/content/www/us/en/developer/articles/technical/software-occlusion-culling.html?wapkw=Occlusion%20culling



### 摘要

本文详细介绍了软件遮挡剔除的算法和相关的示例代码，可以下载。该技术将场景对象划分为遮挡物和遮挡物，并根据与被软件栅格化到深度缓冲区的遮挡物的深度比较来剔除遮挡物。示例代码使用截锥体筛选，并使用流式SIMD扩展(SSE)指令集和多线程进行优化，与示例场景的非筛选显示相比，可以实现高达8倍的性能加速。

### 软件遮挡剔除

遮挡剔除样本已经用Intel®Advanced Vector Extensions 2 (Intel®AVX2)指令（？）进行了优化，以改善剔除过程中花费的时间。英特尔®酷睿™m7-6Y75处理器是一款双核4.5W处理器。表1比较了该处理器上Intel®Streaming SIMD Extensions 4.1 (SSE4.1)版本与AVX2版本的性能。剔除时间随场景和深度缓冲区的分辨率而变化。AVX2减少了1.32倍的剔除时间。

深度缓冲区的分辨率是一个可以改变的参数，以控制遮挡剔除的CPU预算。表1显示，将分辨率从1920x1080降低到320x192可以将时间从7.80 ms降低到1.90 ms。对于为低功耗系统设计的游戏，必须考虑调整这样的参数。m7 6Y75处理器的功耗与平板电脑相同，能够运行Windows* 10等桌面操作系统。因此，任何可以在Windows上运行的游戏都有可能在这些系统上运行，如果游戏内容能够相应地适应较小的功率预算的话。

遮挡剔除通常用于提高游戏性能。虽然这可以在GPU上完成，**但本示例提供了在CPU上运行筛选的替代方法**。与在GPU上剔除几何图形相比，在CPU上剔除几何图形可以带来额外的好处。GPU通常是游戏的瓶颈，在CPU上运行它可以缓解GPU瓶颈，并节省向GPU提交遮挡查询的成本。这个提交成本包括Direct3D* API调用和遮挡器CPU和GPU之间的内存传输。

### 介绍

任何现实的3D场景都包含至少部分遮挡的对象。例如，建筑物和环境通常会遮挡场景中的大量物体。z缓冲区是渲染管道中的最后一个阶段，在这里对象碎片可以被识别为遮挡。然而，被遮挡的物体仍然被发送到GPU进行渲染，没有视觉效果。如果我们可以降低遮挡物体的计算成本，我们可以在不影响场景质量的情况下提高应用程序的性能。

在CPU上使用gpu生成的深度缓冲区是早期检测遮挡对象的一种技术。然而，以这种方式进行遮挡剔除可能会导致延迟和开销问题，并引入视觉伪影，因为GPU可能比CPU慢几帧。为了避免这些问题，我们提出在CPU上对深度缓冲区进行软件光栅化。

在这种方法中，我们使用CPU来栅格化深度缓冲区。然后，我们使用与轴对齐的边界框测试来确定对象是否被遮挡。遮挡的对象从渲染管道中移除以减少开销。我们不会剔除可见和部分可见的物体;我们将它们提交给GPU（？）进行渲染。与Software Occlusion Culling示例相关的示例代码实现了该技术。此外，在这个示例中，软件光栅化器使用SSE和多线程进行矢量化，以提高性能。

occluder:遮挡物

occludee:被遮挡物

软件遮挡剔除分为两个步骤:深度缓冲光栅化和深度测试剔除。接下来的部分将详细介绍这些步骤。

### 深度缓冲区光栅化

光栅化：光栅化就是用光栅设置引擎查找出场景中每个三角形在屏幕空间上的位置，然后对这些像素进行计算。

场景中的遮挡物被栅格化到CPU上的深度缓冲区。图1显示了软件遮挡剔除样本的屏幕截图。城堡的墙壁和地平面在场景中被认为是遮挡物。城堡的墙壁以及附加的微小木制装饰被用作遮挡物，以避免对美术资产进行特殊的预处理。理想情况下，最好是手动选择大型城堡墙壁作为遮挡物，但我们希望确保软件遮挡剔除算法能够在不需要昂贵内容更改的情况下与现实场景一起工作。

我们的栅格化器根据与栅格的交集将帧缓冲区划分为栅格和闭塞三角形。当三角形跨越贴图边界时，光栅化器将三角形分成多个贴图。当处理一个贴图时，一个线程会遍历入盒三角形，并使用边界框遍历将三角形栅格化到深度缓冲区。光栅化器检查所考虑的像素是否在三角形内，如果是，它使用质心坐标在像素位置插入深度值。如果新计算的深度与相同像素位置的现有深度相比更接近观察者，则深度缓冲栅格化过程更新像素位置的深度缓冲。

一旦深度缓冲区在CPU上被栅格化，场景中的所有遮挡都必须针对深度缓冲区进行深度测试，以确定哪些遮挡是可见的，哪些可以剔除。

### 深度测试剔除

我们使用对象空间轴对齐边界框(AABBs)对场景中的遮挡物进行深度测试，以对抗cpu生成的深度缓冲区。深度测试算法将场景中的所有物体(包括遮挡物(城堡墙壁和地平面))视为遮挡物。采用AABB使深度测试更加保守。如果AABB被遮挡，那么它所包含的对象也被遮挡，可以被剔除。如果AABB是可见的，那么假定其中包含的对象也可能是可见的。然而，由于边界框是保守的，这个假设可能并不总是正确的，我们可能会有一些误报。

理想情况下，如果相机在AABB内，则应该剪切包围框，并处理相机前面的物体部分。但是，我们在示例中没有实现clipper阶段。因此，如果任何封闭边界框被附近的剪辑平面剪切，那么我们就会接受封闭为可见并将其发送给GPU进行渲染。

为了确定一个对象是否被近剪切平面剪切，我们可以使用构成边界框的顶点的齐次坐标w。在示例中，近剪辑平面设置为1.0。在相机前面的物体w> 1.0。因此，如果构成边界框的任何顶点的w < 1.0，则该对象正在被近剪辑平面剪切。

然而，我们存储1/w而不是同质坐标w，以避免在光栅化器中的多个位置除以w。

如果$0<w<1$,那么$\frac{1}{w}>1$.然而如果$w<0$那么$\frac{1}{w}<0$.因此，如果1/w > 1.0或1/w < 0.0的边界框的任何顶点的封闭框被剪切，在这种情况下，我们平凡地接受封闭为可见

当截锥体剔除被启用时，AABB深度测试算法处理完全在视界内或被视界剪切的遮挡物。这意味着相机后面的遮挡物被截锥体剔除而不渲染。该示例允许在截锥体剔除被禁用时启用深度缓冲区剔除。这在实践中没有用处，但是我们提供了这个特性来衡量仅通过截锥体剔除获得的性能增益。我们想指出，我们在这个配置中有一个已知的错误;我们微不足道地接受相机后面的所有物体。因此，当禁用截锥体剔除和启用深度测试剔除时，绘制调用的数量会增加。

当一个对象可见并且其边界框的第一个像素通过深度测试时，我们的深度测试算法将该对象标记为可见并将其发送给GPU进行渲染。但是，当物体和边界框完全遮挡时，深度测试算法将测试边界框的所有像素。简而言之，深度测试算法在对象提交给GPU进行渲染时花费的时间最少，而在对象被剔除时执行的工作量最多。

### 优化



### 运行软件遮挡剔除实例



## 代码

#### wWinMain

ParseCommandLine();

中间一些参数设置(?)

实例化一个例子MySample

#### ParseCommandLine();

- wcsicmp:比较两个字符串的大小关系

通过比较argv[i],argv[i+1]和一些字符串的大小，在不同的大小关系的情况下设定一些参数。

gSOCType，gOccludeeSizeThreshold，gDepthTestTasks
