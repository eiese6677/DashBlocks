# Stage 1: Build the React frontend
FROM node:20-slim AS frontend-builder
WORKDIR /app

# Copy package files and install dependencies
# This is separated to leverage Docker layer caching
COPY package.json package-lock.json ./
RUN npm install

# Copy the rest of the frontend source code
COPY index.html vite.config.ts eslint.config.js ./
COPY src ./src
COPY public ./public

# Build the frontend
RUN npm run build

# Stage 2: Create the final Python image
FROM python:3.11-slim AS final-stage
WORKDIR /app

# Install g++ for C++ compilation
RUN apt-get update && apt-get install -y g++ && rm -rf /var/lib/apt/lists/*

# Install Python dependencies
# Copy requirements file first to leverage Docker layer caching
COPY server/requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# Copy server code
COPY server/ ./server/

# Compile the C++ game logic into a shared library
# The sed command is used to remove Windows-specific dllexport attribute
RUN sed -i 's/__declspec(dllexport)//g' server/game_logic.cpp && \
    g++ -shared -fPIC -o server/game_logic.so server/game_logic.cpp

# Copy the built frontend from the builder stage
COPY --from=frontend-builder /app/dist ./dist

# Expose the port the app runs on
EXPOSE 5000

# Set the command to run the application
CMD ["python", "server/app.py"]