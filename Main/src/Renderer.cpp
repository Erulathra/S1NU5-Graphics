#include "Renderer.h"

#include "Buffer.h"
#include "Camera.h"
#include "LinearMath.h"
#include "Mesh.h"
#include "Renderable.h"
#include "TGAWriter.h"
#include "spdlog/spdlog.h"

using namespace YAM;

namespace YAR{
    Renderer::Renderer(uint32_t sizeX, uint32_t sizeY)
        : colorBufferMutex()
          , samplesPerPixel(8)
          , bVariableSamplesPerPixel(true)
          , tilesPerRow(8) {
        colorBuffer = std::make_unique<YAR::Buffer>(sizeX, sizeY);
    }

    Renderer::~Renderer() = default;

    void Renderer::AddRenderable(const std::shared_ptr<Renderable>& renderable) {
        renderables.push_back(renderable);
    }

    void Renderer::Render(const YAR::Camera* camera) const {
        colorBuffer->FillColor(0xff000000);

        const uint32_t tilesNum = tilesPerRow * tilesPerRow;
        std::vector<uint32_t> numbers;
        for (int i = 0; i < tilesNum; ++i) {
            numbers.push_back(i);
        }

        std::atomic<uint32_t> finishedTiles = 0;
        
#pragma omp parallel for
        for (int tileID = 0; tileID < tilesNum; ++tileID) {
            const uint32_t tileY = tileID / tilesPerRow;
            const uint32_t tileX = tileID - tileY * tilesPerRow;

            RenderBounds renderBounds{};
            renderBounds.minX = tileX * colorBuffer->GetSizeX() / tilesPerRow;
            renderBounds.minY = tileY * colorBuffer->GetSizeY() / tilesPerRow;
            renderBounds.maxX = renderBounds.minX + colorBuffer->GetSizeX() / tilesPerRow;
            renderBounds.maxY = renderBounds.minY + colorBuffer->GetSizeY() / tilesPerRow;

            RenderWorker(camera, renderBounds);
            
            ++finishedTiles;
            spdlog::info("Progress: {}%", 100.f * static_cast<float>(finishedTiles) / tilesNum);
        }
    }

    void Renderer::Save(const std::string& path) const {
        TGAWriter::Write(path, colorBuffer->GetData(), colorBuffer->GetSizeX(), colorBuffer->GetSizeY());
    }

    void Renderer::RenderWorker(const Camera* camera, const RenderBounds& renderBounds) const {
        std::vector<Vector3> samples;
        samples.resize(samplesPerPixel);
        
        for (uint32_t i = renderBounds.minY; i < renderBounds.maxY; ++i) {
            for (uint32_t j = renderBounds.minX; j < renderBounds.maxX; ++j) {
                for (Vector3& sample : samples) {
                    sample = Vector3{0.f};
                }

                uint32_t numSamples = 0;
                for (uint32_t sampleID = 0; sampleID < 2; ++sampleID) {
                    samples[sampleID] = SamplePixel(camera, i, j);
                    numSamples++;
                }

                if (!bVariableSamplesPerPixel || (samples[0] - samples[i]).Length() > SmallFloat) {
                    for (int sampleID = 2; sampleID < numSamples; ++sampleID) {
                        samples[sampleID] = SamplePixel(camera, i, j);
                        numSamples++;
                    }
                }

                Vector3 finalColor = Vector3{0};
                for (uint32_t sampleID = 0; sampleID < numSamples; ++sampleID) {
                    finalColor += samples[sampleID];
                }
                
                {
                    std::scoped_lock lock{colorBufferMutex};
                    colorBuffer->SetPix(j, i, Color::FromVector(finalColor / static_cast<flt>(numSamples)).hex);
                }
            }
        }
    }
    
    Vector3 Renderer::SamplePixel(const Camera* camera, uint32_t y, uint32_t x) const {
        const Ray ray = camera->GetRay(x, y);

        RenderHitInfo closestHit;
        closestHit.distance = std::numeric_limits<float>::max();

        bool wasIntersection = false;

        for (const std::shared_ptr<Renderable>& renderable : renderables) {
            if (RenderHitInfo hitInfo; renderable->Trace(ray, hitInfo)) {
                if (hitInfo.distance < closestHit.distance) {
                    closestHit = hitInfo;
                    wasIntersection = true;
                }
            }
        }

        if (wasIntersection) {
            const Vector3 lightDir = {-1.f, -1.f, 1.f};

            const Vector3 objColor = closestHit.material->color.ToVector();
            const float lightDotNorm = std::max(0.f, Vector3::Dot(closestHit.normal, -lightDir.Normal()));
            const float lightValue = std::min(1.f, lightDotNorm + 0.1f);
            return objColor * lightValue;
        }

        return Vector3{0};
    }


    RenderBounds::RenderBounds()
        : minX(0)
          , minY(0)
          , maxX(0)
          , maxY(0) {}
} // SG
