# Plinko-drawer

A C++ tool for visualising the game of Plinko, where small balls or discs are dropped from the top of a board with obstacles such as pegs and other objects. It allows users to draw a custom plinko board of their choosing and simulate a game with realistic ball physics. The board is divided into 20 "buckets" at the bottom which will display a score depending on the settings:

* __Balls Dropped__: Determines how many balls are on the board at a time. Balls have collision logic with drawn walls and eachother. More balls can decrease the time before results, however introduces more randomness into the simulation.

* __Expected Loss__: Determines what you want the average loss to be. Note that this value will be negative as it is meant to simulate Plinko in the context of casinos, where the house is always favoured.

* __Variation__: How varied you want the scores of the buckets to be. For example, setting the variation to 0 will result in all the buckets being equal to the mean. Recommended to set this to a high number.

## To Run:
```console
git clone https://github.com/ianjeong1/plinko-drawer.git
cd plinko-drawer/build
./plinko_drawer
```

## Example Usage:

https://youtu.be/HdmuoIrsNj4

Developed by Ian Jeong 2025