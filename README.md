# Rope-Data-Structure-C

## Description

Implementation of a data structure that can store a string and efficiently cut a part
(a substring) of this string and insert it in a different position.

The current implementation only processes a given string.
It doesn't support insertion of new characters in the string. 

[Wikipedia page on Rope](https://en.wikipedia.org/wiki/Rope_(data_structure))

Uses [Splay tree](https://en.wikipedia.org/wiki/Splay_tree) to implement the Rope data structure.

## Implementation

Nodes don't have keys. They only have values. And the value is a character.  
That means that one node contains and represents a single character.

This data structure is about strings. The string represents (is) contents of a text document.  
In a string, characters are in order, naturally. The order is represented by their rank. That's why we use
order statistics to locate a node when searching for it (when performing a "find" operation).

The rank can be seen as their index, and we'll use 0-based indexing.

The size of a node doesn't have anything to do with its rank. Also, when a node is splayed, its rank doesn't
change; only its size changes.

One has to think in terms of node rank, and not in terms of node key, as is usual!

We could also add the field "rank" to Node objects.  
But, that's not needed. In-order traversal yields all characters in order.
