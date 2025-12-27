docker stop dashblocks-game;
docker rm dashblocks-game;
docker build --no-cache -t dashblocks .;
docker run -d -p 5000:5000 --name dashblocks-game dashblocks;
Write-Output "실행중 : http://localhost:5000/"