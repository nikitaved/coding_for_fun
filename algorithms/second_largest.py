class Node:
    def __init__(self, value=None):
        self.value = value
        self.left = None
        self.right = None


def second_largest(nums):
    """ Finds second largest element in an array
        in O(N + logN) time and O(N) space
    """

    def build_tree(vals):
        n = len(vals)
        if n == 1:
            return Node(vals[0])

        root = Node()
        root.left = build_tree(vals[:n//2])
        root.right = build_tree(vals[n//2:])
        if root.left.value > root.right.value:
            root.value = root.left.value
        else:
            root.value = root.right.value
        return root

    root = build_tree(nums)
    second_largest_elem = -root.value
    while root.left and root.right:
        if root.left.value > root.right.value:
            second_largest_elem = max(root.right.value, second_largest_elem)
            root = root.left
        else:
            second_largest_elem = max(root.left.value, second_largest_elem)
            root = root.right
    return second_largest_elem


if __name__ == '__main__':
    import random
    for n in [100, 1000, 10000, 100000]:
        for i in range(5):
            arr = [random.randint(0, n) for i in range(n)]
            sl = second_largest(arr)
            arr.sort()
            assert sl == arr[-2]
