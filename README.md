--------------------------
**Deadline預警**

ADM Project 3: **2018/04/15**

--------------------------
**留言板**

**置頂**: 來週也要 **Drop\~Drop\~** 到FTP上哦，務必

2018/04/10: 編譯含靜態鏈接庫的文件
          
            mpicc -c MyMPI.c
            
            ar -rc MyMPI.a MyMPI.o
            
            mpicc matrix.c -o matrix ./MyMPI.a

2018/04/08: 如何進行矩陣乘法？**New!**  

2018/04/08: **特報** Octave竟然出了圖形界面

2018/04/06: 發佈了過濾2,3,5,7倍數的程序(prime7b.c)。

2018/04/05: 完成另一種方法編寫去除偶數的方法(prime2b.c)

2018/04/03: **特報** Juno + Mircosoft Azure Network: 擺脫電腦 在IPAD上運行JUPYTER NOTEBOOK

2018/04/02: 修正了Comm_size==1时的严重bug; 发布了过滤2、3、5倍数的版本(prime5.c)

2018/??/??: Template for LaTeX has been uploaded. Please download the folder directly. **Compile the .tex file with other three files**

2018/??/??: 如何建立免密的SSH Connection: 首先在两台电脑上建立名字一样的用户, 然后交换生成的公钥到对方authorized_keys中.

------------------------------

Sharing a Folder

gedit /etc/exports

/mirror *(rw,sync)

-------------------------------

More Details:

http://blog.csdn.net/bendanban/article/details/9136755

http://blog.sina.cn/dpool/blog/s/blog_4e8bda0c0102vqfx.html

-------------------------------
How to compile?

mpicc *.c -o *

mpiexec -n 16 ./*

------------------------------

Hello_world.c can be used as a template for programming

------------------------------

Principe de cpi.c:

Intergrate(4/(1+x^2),x,0,1)=4*(arctan(1)-arctan(0))=pi

------------------------------

