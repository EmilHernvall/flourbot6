function binarySearch(arr, val)
{
    var lower = 0;
    var upper = arr.length;
    var idx = 0;
    while (lower != upper) {
        idx = Math.floor((lower + upper) / 2);
        if (lower == idx && arr[idx] != val) {
            break;
        }

        if (arr[idx] == val) {
            return idx;
        } else if (arr[idx] > val) {
            upper = idx;
        } else {
            lower = idx;
        }
    }

    return -1;
}

