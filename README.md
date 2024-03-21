# livemem - PyTorch Memory Allocation Chunk Analyzer 

Download

```
git clone https://github.com/3cityrnd/livemem.git
```

Build

```
cd livemem
mkdir build && cd build
cmake .. && make
```

Install Torch-CPU & DGL

```
pip install torch torchvision torchaudio --index-url https://download.pytorch.org/whl/cpu
pip install dgl
```


Launch 

```
git clone --depth 1 https://github.com/dmlc/dgl.git

LD_PRELOAD=${LIVEMEM}/build/lib/liblivemem.so python ./dgl/examples/pytorch/graphsage/train_full.py --dataset pubmed 

```

```
[Training with DGL built-in GraphSage module
  NumNodes: 19717
  NumEdges: 88651
  NumFeats: 500
  NumClasses: 3
  NumTrainingSamples: 60
  NumValidationSamples: 500
  NumTestSamples: 1000
Done loading data from cached files.
Training...
Epoch 00000 | Loss 1.1014 | Accuracy 0.3240 
Epoch 00001 | Loss 1.0937 | Accuracy 0.4440 
Epoch 00002 | Loss 1.0842 | Accuracy 0.5060 
Epoch 00003 | Loss 1.0721 | Accuracy 0.6380 
Epoch 00004 | Loss 1.0588 | Accuracy 0.6760 
...
...
...
Epoch 00195 | Loss 0.1966 | Accuracy 0.8040 
Epoch 00196 | Loss 0.1212 | Accuracy 0.8040 
Epoch 00197 | Loss 0.1091 | Accuracy 0.8040 
Epoch 00198 | Loss 0.1481 | Accuracy 0.8040 
Epoch 00199 | Loss 0.1317 | Accuracy 0.8040 
Testing...
Test accuracy 0.7920
+-----------chunk-------|----------count---------|---------------payload------------+
                    1000|                       1|                              1000|
------------------------------------------------------------------------------------|
                     500|                     200|                            100000|
------------------------------------------------------------------------------------|
                    2000|                     200|                            400000|
------------------------------------------------------------------------------------|
                  236604|                    2605|                         616353420|
------------------------------------------------------------------------------------|
                   78868|                    2406|                         189756408|
------------------------------------------------------------------------------------|
                 1261888|                    4006|                        5055123328|
------------------------------------------------------------------------------------|
                       4|                    5006|                             20024|
------------------------------------------------------------------------------------|
                39434000|                     600|                       23660400000|
------------------------------------------------------------------------------------|
                      12|                     803|                              9636|
------------------------------------------------------------------------------------|
                      64|                     803|                             51392|
------------------------------------------------------------------------------------|
                    6000|                     200|                           1200000|
------------------------------------------------------------------------------------|
                   19717|                       3|                             59151|
------------------------------------------------------------------------------------|
                       0|                    1811|                                 0|
------------------------------------------------------------------------------------|
                     480|                     801|                            384480|
------------------------------------------------------------------------------------|
                    8000|                       6|                             48000|
------------------------------------------------------------------------------------|
                    4000|                    1002|                           4008000|
------------------------------------------------------------------------------------|
                   88651|                       1|                             88651|
------------------------------------------------------------------------------------|
                     720|                     800|                            576000|
------------------------------------------------------------------------------------|
                      24|                       2|                                48|
------------------------------------------------------------------------------------|
                       8|                    4611|                             36888|
------------------------------------------------------------------------------------|
                     192|                     803|                            154176|
------------------------------------------------------------------------------------|
                  709184|                       1|                            709184|
------------------------------------------------------------------------------------|
                  157736|                       1|                            157736|
------------------------------------------------------------------------------------|
                  709208|                       2|                           1418416|
------------------------------------------------------------------------------------|
                   32000|                     803|                          25696000|
------------------------------------------------------------------------------------|
                   12000|                       1|                             12000|
------------------------------------------------------------------------------------|
                  866920|                       2|                           1733840|
------------------------------------------------------------------------------------|
[INFO-LIVEMEM]: Report created in livemem_10634.txt
[INFO-LIVEMEM]: JSON data: livemem_10634.json
[INFO-LIVEMEM]: Close dlhandler! 0x21483f0

```

Show chart

```
python livemem/graph/histogram.py --file livemem_10634.json

```

- GAT

![rgat_train](https://github.com/3cityrnd/livemem/assets/152857609/a215100d-8837-4bb4-b99f-b97d996bf163)


- GrahSage Node classification full graph

![graph_sage_node_classification](https://github.com/3cityrnd/livemem/assets/152857609/2f1d9913-2b76-4896-bbb7-a7012c4e4a3e)


