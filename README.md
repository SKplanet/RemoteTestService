
Test Client
사용자가 사용하는 Java Application으로 src 디렉토리 밑에 images resource 디렉토리를 비롯해 6개의 디렉토리가 있음

ftp
단말과의 파일 송수신 기능 제공

images
TC UI에 필요한 이미지와 아이콘

main
TC 구동을 위한 main 모듈

rui
원격지 단말 제어를 위한 이벤트 및 네트워크 Interface 제공

ruiUI
TC 인터페이스 UI 관련 모듈 

utils
데이터의 암호화 전송 등 부가 기능 제공
•	Encryption Logic은 AES 128bit 사용
•	Capture 저장 및 파일 송수신을 위한 디렉토리 설정 

TC는 웹을 통해 Java Web Starter 방식에 따라 배포되며 이를 위한 jnlp 파일 셋팅이 별도로 필요합니다.
웹을 배재 하고 Stand along 형태로 실행하기 위해서는 TC소스코드 에서 직접 TA 가 실행되고 있는 PC의 IP로 접속하시기 바랍니다.

Test Agent
TC 와 단말기 사이를 중계하는 Windows Application이다 TC에 원격제어 요청을 받고 단말기에 설치되어 있는 TD와 통신하여 단말기 화면 버퍼, 리소스 정보를 받아 TC에게 전달한다.

libjpeg
단말기 화면을 JPEG으로 압축하고 해제 하기 위해 사용되는 모듈

NSIS
TA를 인스톨 실드 형태의 설치 파일로 만들기 위한 Installer

rui
원격 제어에 필요한 Core 인터페이스를 제공하는 모듈, 원격제어 필요한 Socket, event, 안드이로이드 ADB 인터페이스를 제공한다.

skrc_adnroid
Android 단말기에 설치되어 단말기의 리소스 정보, 화면 회전 기능을          제공하는 Android 서비스 

skvideo
단말기 화면을 H.264 스트리밍으로 변환하기 위한 인코더/디코더 및 단말기 사운드를 Streaming을 위한 AAC 인코더/디코더를 제공한다. 현재 TC JAVA 버전에서는 H.264 스트리밍 방식은 사용하지 않고 AAC Streaming만 사용한다. AAC 스트리밍 관련해서는 인텔의 IPP(Integrated Performance Primitives) 라이브러리를 사용하며 관련된 라이브러리는 별도로 인텔 사이트에서 다운로드 하여 설치 하여야 한다.


TA
Test Agent Windows 관련 모듈을 제공한다. 각종 Dialog 및 프리뷰 화면 등TA 제어 인터페이스를 제공한다. 내부의 ruicmd 폴더에는 TD 및 Android 서비스,ADB 등 Test Agent 셋업이 필요한 파일이 포함되어 있어 Release시 자동으로 Install shield에 포함된다.


Test Device
Test Device은 단말에 설치되어 단말기 화면 버퍼를 TA에 전송하고 TA에서 전달되는 이벤트를 Android Input Device에 Write하여 Android 이벤트를 발생 시키는 기능을 한다. Android 4.0,4.1,4.2 버전을 지원한다.

ruicapsvc
Android 4.0 버전 TD 소스이다.

ruicapsvcj
Android 4.1 버전 TD소스이다.

ruicapsvcj2
Android 4.2 버전 TD 소스이다.

각 TD 소스는 Android 버전별로 프레임 버퍼를 직접 읽어 오거나 Android Screen capture client를 이용하여 단말기 화면을 얻어 온다.
TD는 Android PDK(Platform Developer’s Kit) 환경에서 빌드는 Android 데몬 형태의 프로그램이다.  

[보안관련 참고사항]
일부 소스에서 Buffer overflow, Format String 공격 등이 발생 될 수 있는 취약한 함수가 사용되고 있어, 악의적인 코드 삽입을 통한 프로그램 오류 발생을 통한 권한 상승 및 시스템 장악 등 추가적인 공격 가능성이 존재합니다. 그 외, 아래 표를 확인하여 취약한 함수를 안전한 함수로 변환하여 사용하시길 권고합니다. 
자세한 내용은 안전행정부에서 제공중인 시큐어코딩 가이드를 참고 하시길 바랍니다.
※ 참고 사이트
http://www.mogaha.go.kr/frt/bbs/type001/commonSelectBoardArticle.do?bbsId=BBSMSTR_000000000012&nttId=42152
