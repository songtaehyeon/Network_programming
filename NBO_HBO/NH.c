#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

uint32_t my_htons(uint32_t value) {
    return ((value & 0xFF) << 24) | ((value & 0xFF00) << 8) | ((value >> 8) & 0xFF00) | ((value >> 24) & 0xFF);
}
//예를 들어 처음 4byte가 01 02 03 04라는 파일이 있다고 해보자
//그럼 2진수로 0000 0001 0000 0010 0000 0011 0000 0100 이 된다.
//처음 (value & 0xFF) << 24 의 과정이다. FF는 1111 1111이다. 처음 바이트인 0000 0100과 and 연산을 하게 되면 똑같은 0000 0100이 나온다
//그것을 24비트 밀어준다. 즉 원래 01 이 있어야 하는 자리에 04를 두는 것이다.
//(value & 0xFF00) << 8 의 경우도 원래 02가 있어야 하는 자리에 03을 두는 것이다.
// (value >> 8) & 0xFF00 의 경우는 살짝 다르다. 먼저 비트를 오른쪽으로 밀어서 04를 지운다음 01 02 03 만 있는 곳에서 FF00과의 연산으로 02를 03이 있어야 할 곳으로 둔다.
//  (value >> 24) & 0xFF 의 경우도 01을 04가 있던 위치에 둔다.
// 그냥 htons를 쓰자

int main(int argc, char *argv[]) {
    uint32_t buf;

    FILE *fp1 = fopen(argv[1], "rb");
    size_t bytesRead = fread(&buf, sizeof(unsigned char), 4, fp1);
    //fread는 파일 스트림에서 원하는 바이트 만큼 원하는 횟수만큼 읽어 버퍼에 저장한다.
    //fread(저장할 버퍼의 주소,읽을 바이트수,읽을 횟수,읽을 파일)
    //즉 fread(&buf, sizeof(unsigned char), 4, fp1); 는 fp1을 읽을 건데 저장할 버퍼는 buf고 1바이트 만큼 4번 읽을 것이다  이라는뜻이다 .
    fclose(fp1);

    uint32_t change = my_htons(buf);
    FILE *fp2 = fopen(argv[2], "wb");

    fwrite(&change, sizeof(uint32_t), 1, fp2);
    //fwrite도 fread랑 같다고 보면 된다. 얘는 쓸려고 파일을 여는 것이다.
    //fwrie(쓰고싶은 데이터가 저장된 메모리의 시작 주소, 쓰고 싶은 데이터의 단위 크기, 반복 횟수, 쓸 파일)
    //즉 fwrite(&change, sizeof(uint32_t), 1, fp2); 는 change라는 데이터를 uint32_t만큼 1번 쓸건데 fp2에 쓸거다 라는 뜻이다.
    fclose(fp2);

    printf("the END\n");

    return 0;
}
