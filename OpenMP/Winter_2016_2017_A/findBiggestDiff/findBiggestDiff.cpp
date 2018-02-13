#include <cstdio>
#include <cstdlib>
#include <omp.h>

#include "node.h"
#include "change_threads_num.h"

typedef int Function(int,int);

static int min(int a, int b) { return a < b ? a : b; }
static int max(int a, int b) { return a < b ? b : a; }

static int appyFunctionToTree(node const* root, Function function)
{
    int left;
    int right;
        
    if (change_threads_num(+1))
    {
#pragma omp parallel sections
        {
#pragma omp section
            {
                left = root->left ? appyFunctionToTree(root->left, function) : root->value;
            }
            
#pragma omp section
            {
                right = root->right ? appyFunctionToTree(root->right, function) : root->value;
            }
        }
        
        change_threads_num(-1);
    }
    else
    {
        left = root->left ? appyFunctionToTree(root->left, function) : root->value;
        right = root->right ? appyFunctionToTree(root->right, function) : root->value;
    }
    
    return function(root->value, function(left, right));
}

int findBiggestDiff(node* root1, node* root2) {
    int isMaxToTreeIndexToValue[2][2];
    
#pragma omp parallel
    {
#pragma omp single
        {
            isMaxToTreeIndexToValue[0][0] = appyFunctionToTree(root1, min);
            isMaxToTreeIndexToValue[0][1] = appyFunctionToTree(root2, min);
            isMaxToTreeIndexToValue[1][0] = appyFunctionToTree(root1, max);
            isMaxToTreeIndexToValue[1][1] = appyFunctionToTree(root2, max);
        }
    }
    
    return max(
               max(
                   abs(isMaxToTreeIndexToValue[0][0] - isMaxToTreeIndexToValue[0][1]),
                   abs(isMaxToTreeIndexToValue[1][0] - isMaxToTreeIndexToValue[1][1])
               ),
               max(
                   abs(isMaxToTreeIndexToValue[1][0] - isMaxToTreeIndexToValue[0][1]),
                   abs(isMaxToTreeIndexToValue[0][0] - isMaxToTreeIndexToValue[1][1])
               )
    );
}
