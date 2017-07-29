#define ROPE_SPLAY
#ifdef ROPE_SPLAY

//#define DEBUG

/* *** Rope Data Structure *** */

/* Implementation of a data structure that can store a string and efficiently cut a part
 * (a substring) of this string and insert it in a different position.
 * This implementation only processes a given string.
 * It doesn't support insertion of new characters in the string. */

/* https://en.wikipedia.org/wiki/Rope_(data_structure) */

/* Uses Splay tree to implement the Rope data structure */

/* Nodes don't have keys. They only have values. And the value is a character.
 * That means that one node contains and represents a single character.
 * This data structure is about strings. The string represents (is) contents of a text document.
 * In a string, characters are in order, of course. The order is represented by their rank. That's why we use
 * order statistics to locate a node when searching for it (performing a "find" operation).
 * The rank can be seen as their index, and we'll use 0-based indexing.
 * The size of a node doesn't have anything to do with its rank. Also, when a node is splayed, its rank doesn't
 * change; only its size changes.
 * One has to think in terms of node rank, and not in terms of node key, as usual!
 * We could also add the field "rank" to Node objects.
 * But, that's not needed. In-order traversal gives all characters in order. */
 
/* MIT License
 * Copyright (c) 2017 Ivan Lazarevic */

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define S_MAX_LEN 300001                                        // + 1 for '\0'
#define TRUE 1
#define FALSE 0

typedef struct Node Node;

/* Node "class" */
struct Node {
    char value;
    Node *parent, *left, *right;
    unsigned size;
};

/* "constructor" for the Node "class" */
static inline Node *createNode(char value);

typedef struct SplayTree SplayTree;

/* SplayTree "class" */
struct SplayTree {
    Node *root;
    unsigned size;
};

/* "constructor" for the SplayTree "class"
 * Creates an empty splay tree. */
static inline SplayTree *createTree(void);

/* "destructor" for the SplayTree "class"
 * Destroys all individual nodes in a tree, and then the tree itself. */
static void destroyTree(SplayTree *tree);

static inline Node *createNode(char value) {
    Node *node = malloc(sizeof(Node));
    node->value = value;
    node->parent = NULL;
    node->left = NULL;
    node->right = NULL;
    node->size = 1;
    return node;
}

static inline SplayTree *createTree(void) {
    SplayTree *tree = malloc(sizeof(SplayTree));
    tree->root = NULL;
    tree->size = 0;
    return tree;
}

/* We need post-order binary tree traversal to free all nodes.
 * Input is a pointer to a tree.
 * This is a usual post-order binary tree traversal in which visit() conducts freeing a node.
 * This is iterative version, because it's several times faster than recursive.
 * A (little) speed-up can be gained by allocating stack statically instead of dynamically. */
static void postOrderFree(SplayTree *tree) {
    unsigned n = tree->size;                                    // unsigned n = tree->size;
    Node **stack = malloc(n * sizeof(*stack));                  // stack contains pointers to nodes
    unsigned size = 0;											// current stack size
    char *boolStack = malloc(n * sizeof(*boolStack));           // For each element on the node stack, a corresponding value is stored on the bool stack. If this value is true, then we need to pop and visit the node on next encounter.
    unsigned boolSize = 0;
    char alreadyEncountered;                                    // boolean
    Node *current = tree->root;
    while (current) {
        stack[size++] = current;
        boolStack[boolSize++] = 0;                              // false
        current = current->left;
    }
    while (size) {
        current = stack[size - 1];
        alreadyEncountered = boolStack[boolSize - 1];
        if (alreadyEncountered) {
            free(current);                                      // visit()
            size--;
            boolSize--;
        }
        else {
            boolSize--;
            boolStack[boolSize++] = 1;                          // true
            current = current->right;
            while (current) {
                stack[size++] = current;
                boolStack[boolSize++] = 0;                      // false
                current = current->left;
            }
        }
    }
    tree->root = NULL;
    tree->size = 0;
    free(stack);
    free(boolStack);
}

static void destroyTree(SplayTree *tree) {
    if (!tree)
        return;
    else if (!tree->root) {
        free(tree);
        return;
    }
    postOrderFree(tree);
    free(tree);
}

/* Iterative in-order traversal.
 * Takes a tree* as input, and returns a string (a pointer to char).
 * It's faster to return (copy) one pointer than the whole string.
 * It could print nodes directly as it traverses the tree (and return void),
 * but that would mean calling putchar() or printf("%c") a large number of times,
 * instead of "appending" to the array result. */
static char *inOrder(SplayTree *tree) {
    Node *current = tree->root;
    /* static, because we want to initialize it with zeros (it'll contain a string), and because we need it outside of this function, in main().
    This is faster than: char *result = calloc(tree->size + 1, sizeof(*result)); */
    static char result[S_MAX_LEN];
    unsigned index = 0;
    if (!current)
        return result;
    Node **stack = malloc(tree->size * sizeof(*stack));
    size_t stackIndex = 0;
    while (TRUE) {
        while (current) {
            stack[stackIndex++] = current;
            current = current->left;
        }
        if (stackIndex) {
            current = stack[--stackIndex];
            result[index++] = current->value;                   // visit()
            current = current->right;
        }
        else
            break;
    }
    free(stack);
    return result;
}

/* Input: Pointer to a tree, and a pointer to its node object that we want to rotate right.
 * Returns nothing.
 * Doesn't splay any node. */
static void _rotateRight(SplayTree *tree, Node *node) {
    Node *parent = node->parent;
    Node *Y = node->left;
    if (!Y)
        return;                                                 // we can't rotate the node with nothing!
    Node *B = Y->right;
    Y->parent = parent;
    if (parent) {
        if (node == parent->left)                               // node is left child
            parent->left = Y;
        else                                                    // node is right child
            parent->right = Y;
    }
    else
        tree->root = Y;

    node->parent = Y;
    Y->right = node;
    if (B)
        B->parent = node;
    node->left = B;

    node->size = (node->left ? node->left->size : 0) + (node->right ? node->right->size : 0) + 1;
    Y->size = (Y->left ? Y->left->size : 0) + (Y->right ? Y->right->size : 0) + 1;
}

/* Input: Pointer to a tree, and a pointer to its node object that we want to rotate left.
 * Returns nothing.
 * Doesn't splay any node. */
static void _rotateLeft(SplayTree *tree, Node *node) {
    Node *parent = node->parent;
    Node *X = node->right;
    if (!X)
        return;                                                 // we can't rotate the node with nothing!
    Node *B = X->left;
    X->parent = parent;
    if (parent) {
        if (node == parent->left)                               // node is left child
            parent->left = X;
        else                                                    // node is right child
            parent->right = X;
    }
    else
        tree->root = X;

    node->parent = X;
    X->left = node;
    if (B)
        B->parent = node;
    node->right = B;

    node->size = (node->left ? node->left->size : 0) + (node->right ? node->right->size : 0) + 1;
    X->size = (X->left ? X->left->size : 0) + (X->right ? X->right->size : 0) + 1;
}

/* Splays node to the top of the tree, making it new root of the tree.
 * Input: Pointer to a tree, and a pointer to its node object that we want to splay to the root.
 * Returns nothing. */
static void _splay(SplayTree *tree, Node *node) {
    if (!node)
        return;

    Node *parent = node->parent;

    while (parent) {

        Node *left = node->left;
        Node *right = node->right;
        Node *grandParent = parent->parent;

        if (!grandParent) {
            /* Zig */
            if (node == parent->left)
                _rotateRight(tree, parent);
            else
                _rotateLeft(tree, parent);
        }

        else if (node == parent->left) {
            if (parent == grandParent->left) {
                /* Zig-zig */
                _rotateRight(tree, grandParent);
                _rotateRight(tree, parent);
            }
            else {
                /* Zig-zag (parent == grandParent.right) */
                _rotateRight(tree, parent);
                _rotateLeft(tree, grandParent);
            }
        }

        else if (node == parent->right) {
            if (parent == grandParent->right) {
                /* Zig-zig */
                _rotateLeft(tree, grandParent);
                _rotateLeft(tree, parent);
            }
            else {
                /* Zig-zag (parent == grandParent.left) */
                _rotateLeft(tree, parent);
                _rotateRight(tree, grandParent);
            }
        }

        parent = node->parent;
    }
}

/* Input: Integer number k - the rank of a node (0 <= k < size of the whole tree).
 * Output: The k - th smallest element in the tree (a node object). Counting starts from 0.
 * This is a public method, which splays the found node to the top of the tree. */
Node *orderStatisticZeroBasedRanking(SplayTree *tree, unsigned k) {
#ifdef DEBUG
    if (k >= tree->size || k < 0) {
        printf("0 <= k < size of the whole tree\n");
        exit(-1);
    }
#endif // DEBUG
    Node *node = tree->root;
    while (node) {
        Node *left = node->left;
        Node *right = node->right;
        unsigned s = left ? left->size : 0;
        if (k == s)
            break;
        else if (k < s) {
            if (left) {
                node = left;
                continue;
            }
            break;
        }
        else {
            if (right) {
                k = k - s - 1;
                node = right;
                continue;
            }
            break;
        }
    }
    _splay(tree, node);
    return node;
}

/* We don't use key. We instead use rank as the position at which to insert a letter (node). */
/* Input: rank is a numerical value (0 <= rank <= size of the whole tree); value is a lowercase English letter.
 * This is a general splay tree method, that works in general case.
 * Adds a node with letter "value" to the tree (string), at the position "rank". Numbering is 0-based.
 * Splays the node up to the top of the tree.
 * But, if we insert more than one character at one time (a string), it will also work. It will accept a string, and put it in a node.
 * Returns nothing.
 * Goes down from root to a leaf only once, and also goes up only once. */
static void insert(SplayTree *tree, unsigned rank, char value) {
#ifdef DEBUG
    if (rank > tree->size || rank < 0) {
        printf("0 <= rank <= size of the whole tree\n");
        exit(-1);
    }
#endif // DEBUG

    Node *node = createNode(value);

    /* Inserting at the end of the whole text. */
    if (rank == tree->size && tree->size > 0) {
        Node *last = orderStatisticZeroBasedRanking(tree, rank - 1);    // Or, subtreeMaximum(tree, root)
        node->left = last;
        node->size = last->size + 1;
        last->parent = node;
        tree->size++;                                                   // Tree size
        tree->root = node;
        return;
    }

    /* Inserting in the middle (or at the beginning). */
    if (tree->size == 0) {
        /* The tree is empty. */
        tree->size++;                                                   // Tree size
        tree->root = node;
        return;
    }
    Node *right = orderStatisticZeroBasedRanking(tree, rank);           // This will be right node of the newly inserted node.
    node->right = right;
    node->left = right->left;
    right->parent = node;
    right->left = NULL;
    right->size = (right->right ? right->right->size : 0) + 1;
    node->size = (node->left ? node->left->size : 0) + (node->right ? node->right->size : 0) + 1;
    tree->size++;                                                       // Tree size
    tree->root = node;
}

/* Input: value is a lowercase English letter.
 * This is a specific method, that doesn't work in general case of a splay tree.
 * Namely, we first insert entire string and then perform operations on it.
 * We will never insert a new character in the string again.
 * Adds a node with letter "value" to the tree, as the new root of the tree.
 * Returns nothing. */
static void insertSpecific(SplayTree *tree, char value) {
    Node *node = createNode(value);
    if (tree->root)
        tree->root->parent = node;
    node->left = tree->root;
    node->size = (node->left ? node->left->size : 0) + 1;
    tree->root = node;
    tree->size++;
}

/* Input: pointer to a tree; pointer to a Node object in the tree.
 * Returns a pointer to a node object with maximum key value in the subtree rooted at node.
 * Splays the found node to the top of the tree. */
static Node *subtreeMaximum(SplayTree *tree, Node *node) {
    if (!node)
        return NULL;
    while (node->right)
        node = node->right;
    _splay(tree, node);
    return node;
}



/* Merges two Splay trees, tree1 and tree2, using the last element (of highest rank) in tree1 (left string) as the node for merging, into a new Splay tree.
 * CONSTRAINTS: None.
 * INPUT: pointers to tree1 and tree2.
 * OUTPUT (the return value of this function) is pointer to tree1, with all the elements of both trees.
 * USAGE: After this function, we can delete tree2. */
static SplayTree *merge(SplayTree *tree1, SplayTree *tree2) {
    if (!tree1 || !tree1->root)
        return tree2;
    if (!tree2 || !tree2->root)
        return tree1;
    Node *root2 = tree2->root;
    Node *root1 = subtreeMaximum(tree1, tree1->root);
    root2->parent = root1;
    root1->right = root2;
    root1->size = (root1->left ? root1->left->size : 0) + (root1->right ? root1->right->size : 0) + 1;
    tree1->size = root1->size;
    return tree1;
}

/*  Splits Splay tree into two trees.
 * Input: pointer to a Splay tree; rank of a node (counting starts from 0; 0 <= rank < size of the whole tree);
 *     two pointers to SplayTree pointers, by which the new Splay trees are returned (in-out).
 * Output: Two Splay trees, one with elements with rank <= "rank", the other with elements with rank > "rank",
 *     fetched by the last two arguments to the function.
 * There is no return value. */
static void split(SplayTree *tree, unsigned rank, SplayTree **tree1, SplayTree **tree2) {
    Node *root1 = orderStatisticZeroBasedRanking(tree, rank);
    Node *root2 = root1->right;
    root1->right = NULL;
    root1->size = (root1->left ? root1->left->size : 0) + (root1->right ? root1->right->size : 0) + 1;
    *tree1 = createTree();
    (*tree1)->root = root1;                                     // insertTree()
    (*tree1)->size = root1->size;
    *tree2 = createTree();
    if (root2) {
        root2->parent = NULL;
        (*tree2)->root = root2;                                 // insertTree()
        (*tree2)->size = root2->size;
    }
    return;
}

/* This is cut-and-paste function.
 * For i and j, counting starts from 0; for k, counting starts from 1.
 * We paste the substring after the k - th symbol of the remaining string(after cutting).
 * If k == 0, we insert the substring at the beginning. */
/* We can't destroy left, middle and right at the end of the function,
 * because merge() doesn't create a new tree.
 * Function merge() returns a pointer to a tree, which means that its two parts
 * are preserved. It just merges them together.
 * Globally, we don't change the size of the tree in this function.
 * We just split it and then merge it back.
 * That's why total memory consumption is fine. */
void process(SplayTree **tree, unsigned i, unsigned j, unsigned k) {
    /* If these three pointers are declared static, it's very slow. */
    SplayTree *left = NULL, *middle = NULL, *right = NULL;
    split(*tree, j, &middle, &right);
    if (i > 0)
        split(middle, i - 1, &left, &middle);
    left = merge(left, right);
    if (k > 0)
        split(left, k - 1, &left, &right);
    else {
        right = left;
        left = NULL;
    }
    *tree = merge(merge(left, middle), right);
    return;
}


/*
 * Example usage:
 * Input a string S from a line.
 * The next line contains number of operations that we want to perform on the string, numOps.
 * The following numOps lines contain triples of integers (i, j, k).
 * For i and j, counting starts from 0; for k, counting starts from 1.
 * The code will cut the substring S[i..j] from S and insert it after the k-th character of
 * the remaining string. If k == 0, it inserts the substring at the beginning.
 * Constraints:
 * 0 <= i <= j <= n - 1
 * 0 <= k <= n - (j - i + 1)
 */

int main() {
    static char rope[S_MAX_LEN];
    unsigned numOps, n;
    scanf("%s", &rope);
    n = strlen(rope);
    SplayTree *tree = createTree();
    for (size_t i = 0; i < n; i++)
        //insert(tree, i, rope[i]);
        insertSpecific(tree, rope[i]);
    scanf("%u", &numOps);
    for (unsigned i = 0; i < numOps; i++) {
        unsigned i, j, k;
        scanf("%u%u%u", &i, &j, &k);
        process(&tree, i, j, k);
    }
    printf(inOrder(tree));
    destroyTree(tree);

    char c = getchar();
    c = getchar();
    return 0;
}

#endif // ROPE_SPLAY
