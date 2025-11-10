#include "Tree.h"

int main() {
    Tree_t* tree = TreeCtor( 50 );

    tree->root->right = NodeCreate( 12, tree->root );
    tree->root->left = NodeCreate( 11, tree->root );

    tree->root->right->right = NodeCreate( 12, tree->root );

    TreeDump( tree );

    TreeDtor( &tree );
}

